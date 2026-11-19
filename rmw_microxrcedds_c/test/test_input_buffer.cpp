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

#include <gtest/gtest.h>

#include <rmw/rmw.h>
#include <rmw_microxrcedds_c/config.h>

#include <vector>

// The internal headers have no C linkage guards
extern "C"
{
#include "./rmw_microros_internal/memory.h"
#include "./rmw_microros_internal/types.h"
}

/*
 * Testing the order of the static input buffers.
 * These tests use the buffer pool directly, so they need no Agent.
 */

class InputBufferTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    rmw_uxrce_init_static_input_buffer_memory(
      &static_buffer_memory, custom_static_buffers, RMW_UXRCE_MAX_HISTORY);
    static_buffer_memory.is_dynamic_allowed = false;
  }

  void TearDown() override
  {
    rmw_uxrce_mempool_item_t * item = static_buffer_memory.allocateditems;
    while (item != NULL) {
      rmw_uxrce_mempool_item_t * next = item->next;
      put_memory(&static_buffer_memory, item);
      item = next;
    }
  }

  // Stores a sample as the topic, request and reply callbacks do
  void store(void * owner, size_t depth, uint8_t value, int64_t timestamp)
  {
    rmw_qos_profile_t qos = rmw_qos_profile_default;
    qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
    qos.depth = depth;

    rmw_uxrce_mempool_item_t * item = rmw_uxrce_get_static_input_buffer_for_entity(owner, qos);
    ASSERT_NE(item, nullptr);

    rmw_uxrce_static_input_buffer_t * data =
      reinterpret_cast<rmw_uxrce_static_input_buffer_t *>(item->data);
    data->owner = owner;
    data->buffer[0] = value;
    data->length = 1;
    data->timestamp = timestamp;
  }

  // Takes samples as rmw_take does, until the owner has none left
  std::vector<uint8_t> take_all(void * owner)
  {
    std::vector<uint8_t> values;
    rmw_uxrce_mempool_item_t * item;

    while ((item = rmw_uxrce_find_static_input_buffer_by_owner(owner)) != NULL) {
      rmw_uxrce_static_input_buffer_t * data =
        reinterpret_cast<rmw_uxrce_static_input_buffer_t *>(item->data);
      values.push_back(data->buffer[0]);
      put_memory(&static_buffer_memory, item);
    }

    return values;
  }

  uint8_t take(void * owner)
  {
    rmw_uxrce_mempool_item_t * item = rmw_uxrce_find_static_input_buffer_by_owner(owner);
    EXPECT_NE(item, nullptr);
    if (item == NULL) {
      return 0;
    }

    uint8_t value = reinterpret_cast<rmw_uxrce_static_input_buffer_t *>(item->data)->buffer[0];
    put_memory(&static_buffer_memory, item);
    return value;
  }

  int owner_a;
  int owner_b;
};

/*
 * Samples that arrive within one clock tick get the same timestamp.
 */
TEST_F(InputBufferTest, equal_timestamps_keep_arrival_order)
{
  store(&owner_a, 4, 1, 5);
  store(&owner_a, 4, 2, 5);
  store(&owner_a, 4, 3, 5);

  ASSERT_EQ(take_all(&owner_a), (std::vector<uint8_t>{1, 2, 3}));
}

TEST_F(InputBufferTest, overwrites_keep_arrival_order)
{
  for (uint8_t i = 1; i <= 6; i++) {
    store(&owner_a, 2, i, i);
  }

  ASSERT_EQ(take_all(&owner_a), (std::vector<uint8_t>{5, 6}));
}

TEST_F(InputBufferTest, equal_timestamps_after_overwrites_keep_arrival_order)
{
  store(&owner_a, 2, 1, 1);
  store(&owner_a, 2, 2, 2);
  store(&owner_a, 2, 3, 3);
  store(&owner_a, 2, 4, 3);
  store(&owner_a, 2, 5, 3);

  ASSERT_EQ(take_all(&owner_a), (std::vector<uint8_t>{4, 5}));
}

TEST_F(InputBufferTest, overwrite_evicts_the_oldest_on_equal_timestamps)
{
  store(&owner_a, 2, 1, 1);
  store(&owner_a, 2, 2, 2);
  store(&owner_a, 2, 3, 3);
  store(&owner_a, 2, 4, 3);
  store(&owner_a, 2, 5, 3);
  store(&owner_a, 2, 6, 3);

  ASSERT_EQ(take_all(&owner_a), (std::vector<uint8_t>{5, 6}));
}

/*
 * The timestamp is an epoch time, which a time synchronization can move.
 */
TEST_F(InputBufferTest, clock_jumps_do_not_reorder)
{
  store(&owner_a, 3, 1, 9);
  store(&owner_a, 3, 2, 1);
  store(&owner_a, 3, 3, 5);

  ASSERT_EQ(take_all(&owner_a), (std::vector<uint8_t>{1, 2, 3}));
}

/*
 * All entities share one list of buffers.
 */
TEST_F(InputBufferTest, interleaved_owners_keep_their_own_order)
{
  store(&owner_a, 2, 1, 5);
  store(&owner_b, 2, 11, 5);
  store(&owner_a, 2, 2, 5);
  store(&owner_b, 2, 12, 5);
  store(&owner_a, 2, 3, 5);
  store(&owner_b, 2, 13, 5);

  ASSERT_EQ(take_all(&owner_a), (std::vector<uint8_t>{2, 3}));
  ASSERT_EQ(take_all(&owner_b), (std::vector<uint8_t>{12, 13}));
}

TEST_F(InputBufferTest, take_between_stores_keeps_arrival_order)
{
  store(&owner_a, 4, 1, 5);
  store(&owner_a, 4, 2, 5);
  ASSERT_EQ(take(&owner_a), 1);
  store(&owner_a, 4, 3, 5);

  ASSERT_EQ(take_all(&owner_a), (std::vector<uint8_t>{2, 3}));
}

/*
 * Testing move_to_head on a pool of its own.
 */

class MemoryPoolTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    UXR_INIT_LOCK(&pool.mutex);
    for (size_t i = 0; i < 3; i++) {
      put_memory(&pool, &items[i]);
    }

    // Each get_memory inserts at the head
    third = get_memory(&pool);
    second = get_memory(&pool);
    first = get_memory(&pool);
  }

  // Checks the allocated list in both directions
  void expect_allocated(std::vector<rmw_uxrce_mempool_item_t *> expected)
  {
    std::vector<rmw_uxrce_mempool_item_t *> forward;
    rmw_uxrce_mempool_item_t * last = NULL;
    for (rmw_uxrce_mempool_item_t * item = pool.allocateditems; item != NULL; item = item->next) {
      forward.push_back(item);
      last = item;
    }
    EXPECT_EQ(forward, expected);

    std::vector<rmw_uxrce_mempool_item_t *> backward;
    for (rmw_uxrce_mempool_item_t * item = last; item != NULL; item = item->prev) {
      backward.insert(backward.begin(), item);
    }
    EXPECT_EQ(backward, expected);
  }

  rmw_uxrce_mempool_t pool = {};
  rmw_uxrce_mempool_item_t items[3] = {};

  rmw_uxrce_mempool_item_t * first;
  rmw_uxrce_mempool_item_t * second;
  rmw_uxrce_mempool_item_t * third;
};

TEST_F(MemoryPoolTest, move_to_head_from_the_head)
{
  expect_allocated({first, second, third});

  move_to_head(&pool, first);
  expect_allocated({first, second, third});
}

TEST_F(MemoryPoolTest, move_to_head_from_the_middle)
{
  move_to_head(&pool, second);
  expect_allocated({second, first, third});
}

TEST_F(MemoryPoolTest, move_to_head_from_the_tail)
{
  move_to_head(&pool, third);
  expect_allocated({third, first, second});
}

TEST_F(MemoryPoolTest, move_to_head_keeps_the_free_list)
{
  put_memory(&pool, second);
  move_to_head(&pool, third);
  expect_allocated({third, first});

  ASSERT_EQ(get_memory(&pool), second);
  expect_allocated({second, third, first});
  ASSERT_FALSE(has_memory(&pool));
}
