// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rmw/rmw.h>
#include <rmw/time.h>
#include <rmw_microxrcedds_c/config.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "./rmw_base_test.hpp"
#include "./test_utils.hpp"

using std::chrono::milliseconds;
using std::chrono::steady_clock;

// Wall clock tolerances. The lower bound is what the test actually asserts; the upper bound is
// deliberately loose because CI runs these next to a real Agent under an unpredictable load.
static const int64_t TOLERANCE_MS = 100;

class TestWait : public RMWBaseTest
{
protected:
  void SetUp() override
  {
    RMWBaseTest::SetUp();

    node = rmw_create_node(&test_context, "wait_node", "/ns");
    ASSERT_NE(node, nullptr);

    guard_condition = rmw_create_guard_condition(&test_context);
    ASSERT_NE(guard_condition, nullptr);

    guard_condition_handle = guard_condition->data;
    guard_conditions.guard_conditions = &guard_condition_handle;
    guard_conditions.guard_condition_count = 1;
  }

  void TearDown() override
  {
    ASSERT_EQ(rmw_destroy_guard_condition(guard_condition), RMW_RET_OK);
    ASSERT_EQ(rmw_destroy_node(node), RMW_RET_OK);
    RMWBaseTest::TearDown();
  }

  // rmw_wait() empties the wait set arrays it is given, so they are rebuilt before every call.
  void ResetGuardConditions()
  {
    guard_condition_handle = guard_condition->data;
    guard_conditions.guard_conditions = &guard_condition_handle;
    guard_conditions.guard_condition_count = 1;
  }

  rmw_subscription_t * CreateSubscription()
  {
    ConfigureDummyTypeSupport(
      topic_type, topic_type, message_namespace, id_gen++, &dummy_type_support);

    rmw_qos_profile_t dummy_qos_policies;
    ConfigureDefaultQOSPolices(&dummy_qos_policies);

    rmw_subscription_options_t options = rmw_get_default_subscription_options();

    return rmw_create_subscription(
      node, &dummy_type_support.type_support, topic_name, &dummy_qos_policies, &options);
  }

  // Runs rmw_wait() on a detached thread and gives up after `budget` to avoid hungs
  bool WaitWithBudget(
    rmw_subscriptions_t * subscriptions,
    rmw_guard_conditions_t * gcs,
    const rmw_time_t * timeout,
    milliseconds budget,
    rmw_ret_t * out_ret)
  {
    auto done = std::make_shared<std::atomic_bool>(false);
    auto result = std::make_shared<std::atomic_int>(RMW_RET_ERROR);

    std::thread(
      [done, result, subscriptions, gcs, timeout]() {
        rmw_ret_t rc = rmw_wait(subscriptions, gcs, NULL, NULL, NULL, NULL, timeout);
        result->store(rc);
        done->store(true);
      }).detach();

    const auto deadline = steady_clock::now() + budget;
    while (!done->load() && steady_clock::now() < deadline) {
      std::this_thread::sleep_for(milliseconds(1));
    }

    if (!done->load()) {
      return false;
    }
    *out_ret = static_cast<rmw_ret_t>(result->load());
    return true;
  }

  rmw_node_t * node;
  rmw_guard_condition_t * guard_condition;
  void * guard_condition_handle;
  rmw_guard_conditions_t guard_conditions;

  dummy_type_support_t dummy_type_support;
  const char * topic_type = "topic_type";
  const char * topic_name = "wait_topic";
  const char * message_namespace = "package_name";
  size_t id_gen = 0;
};

/*
 * A wait set holding only guard conditions must block for the requested timeout instead of
 * returning immediately. Returning immediately is what makes rclc spin at 100% CPU when an
 * application only has timers.
 */
TEST_F(TestWait, guard_condition_only_blocks_for_timeout)
{
  const int64_t timeout_ms = 300;
  rmw_time_t timeout = {0LL, timeout_ms * 1000000ULL};

  ResetGuardConditions();
  const auto start = steady_clock::now();
  rmw_ret_t rc = rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout);
  const auto elapsed =
    std::chrono::duration_cast<milliseconds>(steady_clock::now() - start).count();

  EXPECT_EQ(rc, RMW_RET_TIMEOUT);
  EXPECT_GE(elapsed, timeout_ms - TOLERANCE_MS);
}

/*
 * A wait shorter than one internal wait slice must cost about the timeout, not a whole slice.
 * This is what catches a slice that is not clamped to the time remaining. Tested in a loop
 * to ensure the aggregated difference between clamped and unclamped slices is significant
 * enough to be measured reliably.
 */
TEST_F(TestWait, short_timeouts_are_not_rounded_up_to_a_full_slice)
{
  const int iterations = 20;
  const int64_t timeout_ms = 1;
  rmw_time_t timeout = {0LL, timeout_ms * 1000000ULL};

  const auto start = steady_clock::now();
  for (int i = 0; i < iterations; ++i) {
    ResetGuardConditions();
    EXPECT_EQ(
      rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout),
      RMW_RET_TIMEOUT) << "iteration " << i;
  }
  const auto elapsed =
    std::chrono::duration_cast<milliseconds>(steady_clock::now() - start).count();

  // Around 20 ms when every slice is clamped to the time remaining, and around 200 ms when each
  // wait costs a full RMW_UXRCE_MAX_SESSION_WAIT_SLICE_MS regardless of how little is left
  EXPECT_LE(elapsed, 100) << iterations << " waits of " << timeout_ms << " ms took "
                          << elapsed << " ms";
}

/*
 * A guard condition triggered before rmw_wait() is called must be reported without blocking.
 */
TEST_F(TestWait, already_triggered_guard_condition_returns_immediately)
{
  rmw_time_t timeout = {1LL, 0LL};  // 1 second

  ASSERT_EQ(rmw_trigger_guard_condition(guard_condition), RMW_RET_OK);

  ResetGuardConditions();
  const auto start = steady_clock::now();
  rmw_ret_t rc = rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout);
  const auto elapsed =
    std::chrono::duration_cast<milliseconds>(steady_clock::now() - start).count();

  EXPECT_EQ(rc, RMW_RET_OK);
  EXPECT_LT(elapsed, 1000 - TOLERANCE_MS);
  EXPECT_NE(guard_conditions.guard_conditions[0], nullptr);
}

/*
 * A guard condition triggered by another thread while rmw_wait() is blocked must wake the wait
 * up early. rmw_trigger_guard_condition() cannot interrupt an XRCE receive, so the only way to
 * pass this is to poll the flag while waiting.
 */
TEST_F(TestWait, guard_condition_triggered_while_waiting_wakes_up)
{
  const int64_t timeout_ms = 2000;
  const int64_t trigger_after_ms = 200;
  rmw_time_t timeout = {0LL, timeout_ms * 1000000ULL};

  std::thread trigger_thread(
    [this, trigger_after_ms]() {
      std::this_thread::sleep_for(milliseconds(trigger_after_ms));
      EXPECT_EQ(rmw_trigger_guard_condition(this->guard_condition), RMW_RET_OK);
    });

  ResetGuardConditions();
  const auto start = steady_clock::now();
  rmw_ret_t rc = rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout);
  const auto elapsed =
    std::chrono::duration_cast<milliseconds>(steady_clock::now() - start).count();

  trigger_thread.join();

  EXPECT_EQ(rc, RMW_RET_OK);
  EXPECT_GE(elapsed, trigger_after_ms - TOLERANCE_MS);
  EXPECT_LT(elapsed, timeout_ms - TOLERANCE_MS);
}

/*
 * The same as the previous test, but with a subscription in the wait set.
 * This is the ordinary rclc executor shape and it takes a different branch
 * of rmw_wait(): the one that blocks in uxr_run_session_until_data() for
 * every session owning a wait set entity.
 */
TEST_F(TestWait, guard_condition_with_subscription_in_wait_set_wakes_up)
{
  rmw_subscription_t * subscription = CreateSubscription();
  ASSERT_NE(subscription, nullptr);

  const int64_t timeout_ms = 2000;
  const int64_t trigger_after_ms = 200;
  rmw_time_t timeout = {0LL, timeout_ms * 1000000ULL};

  void * subscription_handle = subscription->data;
  rmw_subscriptions_t subscriptions;
  subscriptions.subscribers = &subscription_handle;
  subscriptions.subscriber_count = 1;

  std::thread trigger_thread(
    [this, trigger_after_ms]() {
      std::this_thread::sleep_for(milliseconds(trigger_after_ms));
      EXPECT_EQ(rmw_trigger_guard_condition(this->guard_condition), RMW_RET_OK);
    });

  ResetGuardConditions();
  const auto start = steady_clock::now();
  rmw_ret_t rc = rmw_wait(&subscriptions, &guard_conditions, NULL, NULL, NULL, NULL, &timeout);
  const auto elapsed =
    std::chrono::duration_cast<milliseconds>(steady_clock::now() - start).count();

  trigger_thread.join();

  EXPECT_EQ(rc, RMW_RET_OK);
  // More tight timeout to prove the guard condition woke the wait
  EXPECT_LT(elapsed, trigger_after_ms + 3 * TOLERANCE_MS) << "woke late: " << elapsed << " ms";

  EXPECT_EQ(rmw_destroy_subscription(node, subscription), RMW_RET_OK);
}

/*
 * A sub-millisecond timeout truncates to 0 ms in rmw_wait(). The call must still terminate
 * promptly and must not turn into a spin.
 * With RMW_UXRCE_MAX_SESSIONS forced to 2 for the tests, this is the 1 ms / 2 sessions = 0 case.
 */
TEST_F(TestWait, sub_millisecond_timeout_terminates)
{
  rmw_time_t timeout = {0LL, 500000ULL};  // 500 us

  ResetGuardConditions();
  const auto start = steady_clock::now();
  rmw_ret_t rc = rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout);
  const auto elapsed =
    std::chrono::duration_cast<milliseconds>(steady_clock::now() - start).count();

  EXPECT_EQ(rc, RMW_RET_TIMEOUT);
  EXPECT_LT(elapsed, TOLERANCE_MS);
}

/*
 * A zero timeout is a poll: it must return without blocking, whether or not anything is ready.
 */
TEST_F(TestWait, zero_timeout_returns_immediately)
{
  rmw_time_t timeout = {0LL, 0LL};

  ResetGuardConditions();
  const auto start = steady_clock::now();
  rmw_ret_t rc = rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout);
  const auto elapsed =
    std::chrono::duration_cast<milliseconds>(steady_clock::now() - start).count();

  EXPECT_EQ(rc, RMW_RET_TIMEOUT);
  EXPECT_LT(elapsed, TOLERANCE_MS);
}

/*
 * An infinite timeout must still be woken by a guard condition. Run on a detached thread
 * with a hard budget, so that a regression is reported as a failed assertion rather than
 * hanging.
 */
TEST_F(TestWait, infinite_timeout_wakes_on_guard_condition)
{
  const int64_t trigger_after_ms = 200;

  std::thread trigger_thread(
    [this, trigger_after_ms]() {
      std::this_thread::sleep_for(milliseconds(trigger_after_ms));
      EXPECT_EQ(rmw_trigger_guard_condition(this->guard_condition), RMW_RET_OK);
    });

  ResetGuardConditions();
  rmw_time_t timeout = (rmw_time_t)RMW_DURATION_INFINITE;
  rmw_ret_t rc = RMW_RET_ERROR;
  const bool returned =
    WaitWithBudget(NULL, &guard_conditions, &timeout, milliseconds(5000), &rc);

  trigger_thread.join();

  ASSERT_TRUE(returned) << "rmw_wait() with an infinite timeout was not woken by a guard "
    "condition within 5 s";
  EXPECT_EQ(rc, RMW_RET_OK);
}

/*
 * Repeated waits that time out must not leave the guard condition latched, and must keep
 * reporting RMW_RET_TIMEOUT. This is a cheap regression net around the flag bookkeeping at the
 * end of rmw_wait(), which clears hasTriggered on the way out.
 */
TEST_F(TestWait, repeated_timeouts_are_stable)
{
  rmw_time_t timeout = {0LL, 50000000ULL};  // 50 ms

  for (int i = 0; i < 5; ++i) {
    ResetGuardConditions();
    rmw_ret_t rc = rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout);
    EXPECT_EQ(rc, RMW_RET_TIMEOUT) << "iteration " << i << " returned " << rc;
    EXPECT_EQ(guard_conditions.guard_conditions[0], nullptr) << "iteration " << i;
  }

  ASSERT_EQ(rmw_trigger_guard_condition(guard_condition), RMW_RET_OK);
  ResetGuardConditions();
  EXPECT_EQ(
    rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout), RMW_RET_OK);

  // The trigger must have been consumed by the successful wait above.
  ResetGuardConditions();
  EXPECT_EQ(
    rmw_wait(NULL, &guard_conditions, NULL, NULL, NULL, NULL, &timeout), RMW_RET_TIMEOUT);
}
