// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <vector>
#include <memory>
#include <string>

#include <rosidl_typesupport_microxrcedds_shared/identifier.h>
#include <rosidl_typesupport_microxrcedds_shared/message_type_support.h>

#include "rmw/error_handling.h"
#include "rmw/node_security_options.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "./config.h"
#include "./test_utils.hpp"

class TestSubscription : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    #ifndef _WIN32
    freopen("/dev/null", "w", stderr);
    #endif
  }

  void SetUp()
  {
    rmw_ret_t ret = rmw_init();
    ASSERT_EQ(ret, RMW_RET_OK);

    rmw_node_security_options_t security_options;
    node = rmw_create_node("my_node", "/ns", 0, &security_options);
    ASSERT_NE((void *)node, (void *)NULL);
  }

  rmw_node_t * node;

  const char * topic_type = "topic_type";
  const char * topic_name = "topic_name";
  const char * package_name = "package_name";

  size_t id_gen = 0;
};

/*
   Testing subscription construction and destruction.
 */
TEST_F(TestSubscription, construction_and_destruction) {
  dummy_type_support_t dummy_type_support;
  ConfigureDummyTypeSupport(
    topic_type,
    topic_type,
    package_name,
    id_gen++,
    &dummy_type_support);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  rmw_subscription_t * sub = rmw_create_subscription(
    this->node,
    &dummy_type_support.type_support,
    topic_name,
    &dummy_qos_policies,
    ignore_local_publications);
  ASSERT_NE((void *)sub, (void *)NULL);

  rmw_ret_t ret = rmw_destroy_subscription(this->node, sub);
  ASSERT_EQ(ret, RMW_RET_OK);
}


/*
   Testing node memory poll
 */
TEST_F(TestSubscription, memory_poll_multiple_topic) {
  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  std::vector<dummy_type_support_t> dummy_type_supports;
  std::vector<rmw_subscription_t *> subscriptions;
  rmw_ret_t ret;
  rmw_subscription_t * subscription;


  // Get all available nodes
  {
    for (size_t i = 0; i < MAX_SUBSCRIPTIONS_X_NODE; i++) {
      dummy_type_supports.push_back(dummy_type_support_t());
      ConfigureDummyTypeSupport(
        topic_type,
        topic_type,
        package_name,
        id_gen++,
        &dummy_type_supports.back());
      subscription = rmw_create_subscription(
        this->node,
        &dummy_type_supports.back().type_support,
        dummy_type_supports.back().topic_name.data(),
        &dummy_qos_policies,
        ignore_local_publications);
      ASSERT_NE((void *)subscription, (void *)NULL);
      subscriptions.push_back(subscription);
    }
  }


  // Try to get one
  {
    dummy_type_supports.push_back(dummy_type_support_t());
    ConfigureDummyTypeSupport(
      topic_type,
      topic_type,
      package_name,
      id_gen++,
      &dummy_type_supports.back());
    subscription = rmw_create_subscription(
      this->node,
      &dummy_type_supports.back().type_support,
      dummy_type_supports.back().topic_name.data(),
      &dummy_qos_policies,
      ignore_local_publications);
    ASSERT_EQ((void *)subscription, (void *)NULL);
    ASSERT_EQ(CheckErrorState(), true);
  }


  // Relese one
  {
    subscription = subscriptions.back();
    subscriptions.pop_back();
    ret = rmw_destroy_subscription(this->node, subscription);
    ASSERT_EQ(ret, RMW_RET_OK);
  }


  // Get one
  {
    dummy_type_supports.push_back(dummy_type_support_t());
    ConfigureDummyTypeSupport(
      topic_type,
      topic_type,
      package_name,
      id_gen++,
      &dummy_type_supports.back());
    subscription = rmw_create_subscription(
      this->node,
      &dummy_type_supports.back().type_support,
      dummy_type_supports.back().topic_name.data(),
      &dummy_qos_policies,
      ignore_local_publications);
    ASSERT_NE((void *)subscription, (void *)NULL);
    subscriptions.push_back(subscription);
  }


  // Release all
  {
    for (size_t i = 0; i < subscriptions.size(); i++) {
      subscription = subscriptions.at(i);
      ret = rmw_destroy_subscription(this->node, subscription);
      ASSERT_EQ(ret, RMW_RET_OK);
    }
    subscriptions.clear();
  }
}


/*
   Testing node memory poll for same topic
 */
TEST_F(TestSubscription, memory_poll_shared_topic) {
  dummy_type_support_t dummy_type_support;
  ConfigureDummyTypeSupport(
    topic_type,
    topic_type,
    package_name,
    id_gen++,
    &dummy_type_support);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  std::vector<rmw_subscription_t *> subscriptions;
  rmw_ret_t ret;
  rmw_subscription_t * subscription;


  // Get all available nodes
  {
    for (size_t i = 0; i < MAX_SUBSCRIPTIONS_X_NODE; i++) {
      subscription = rmw_create_subscription(
        this->node,
        &dummy_type_support.type_support,
        dummy_type_support.topic_name.data(),
        &dummy_qos_policies,
        ignore_local_publications);
      ASSERT_NE((void *)subscription, (void *)NULL);
      subscriptions.push_back(subscription);
    }
  }


  // Try to get one
  {
    subscription = rmw_create_subscription(
      this->node,
      &dummy_type_support.type_support,
      dummy_type_support.topic_name.data(),
      &dummy_qos_policies,
      ignore_local_publications);
    ASSERT_EQ((void *)subscription, (void *)NULL);
    ASSERT_EQ(CheckErrorState(), true);
  }


  // Relese one
  {
    subscription = subscriptions.back();
    subscriptions.pop_back();
    ret = rmw_destroy_subscription(this->node, subscription);
    ASSERT_EQ(ret, RMW_RET_OK);
  }


  // Get one
  {
    subscription = rmw_create_subscription(
      this->node,
      &dummy_type_support.type_support,
      dummy_type_support.topic_name.data(),
      &dummy_qos_policies,
      ignore_local_publications);
    ASSERT_NE((void *)subscription, (void *)NULL);
    subscriptions.push_back(subscription);
  }


  // Release all
  {
    for (size_t i = 0; i < subscriptions.size(); i++) {
      subscription = subscriptions.at(i);
      ret = rmw_destroy_subscription(this->node, subscription);
      ASSERT_EQ(ret, RMW_RET_OK);
    }
    subscriptions.clear();
  }
}
