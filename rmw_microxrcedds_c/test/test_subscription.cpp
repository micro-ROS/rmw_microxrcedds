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

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPServerWindows.hpp>
#else
#include <uxr/agent/transport/udp/UDPServerLinux.hpp>
#endif  // _WIN32

#include <rosidl_typesupport_microxrcedds_shared/identifier.h>
#include <rosidl_typesupport_microxrcedds_shared/message_type_support.h>

#include "rmw/error_handling.h"
#include "rmw/node_security_options.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "./config.h"

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

    server =
      std::unique_ptr<eprosima::uxr::Server>(new eprosima::uxr::UDPServer((uint16_t)atoi("8888")));
    server->run();
    // ASSERT_EQ(server->run(), true);

    rmw_node_security_options_t security_options;
    node = rmw_create_node("my_node", "/ns", 0, &security_options);
    ASSERT_NE((void *)node, (void *)NULL);
  }

  void ConfigureDummyTypeSupport(
    rosidl_message_type_support_t * dummy_type_support,
    message_type_support_callbacks_t * dummy_callbacks)
  {
    topic_name[sizeof(topic_name) - 2]++;
    dummy_callbacks->message_name_ = topic_name;
    dummy_callbacks->package_name_ = "test";
    dummy_callbacks->cdr_serialize = [](const void * untyped_ros_message, ucdrBuffer * cdr) {
        return true;
      };
    dummy_callbacks->cdr_deserialize =
      [](ucdrBuffer * cdr, void * untyped_ros_message, uint8_t * raw_mem_ptr, size_t raw_mem_size) {
        return true;
      };
    dummy_callbacks->get_serialized_size = [](const void *) {return (uint32_t)0;};
    dummy_callbacks->max_serialized_size = [](bool full_bounded) {return (size_t)0;};

    dummy_type_support->typesupport_identifier =
      ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE;
    dummy_type_support->data = dummy_callbacks;
    dummy_type_support->func =
      [](const rosidl_message_type_support_t * type_support, const char * id) {
        return type_support;
      };
  }

  void ConfigureDefaultQOSPolices(rmw_qos_profile_t * dummy_qos_policies)
  {
    dummy_qos_policies->avoid_ros_namespace_conventions = false;
    dummy_qos_policies->depth = 0;

    // durability options:
    //  RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT
    //  RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
    //  RMW_QOS_POLICY_DURABILITY_VOLATILE
    dummy_qos_policies->durability = RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT;

    // history options:
    //  RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT
    //  RMW_QOS_POLICY_HISTORY_KEEP_LAST
    //  RMW_QOS_POLICY_HISTORY_KEEP_ALL
    dummy_qos_policies->history = RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT;

    // reliability options:
    //  RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT
    //  RMW_QOS_POLICY_RELIABILITY_RELIABLE
    //  RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT
    dummy_qos_policies->reliability = RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT;
  }

  bool CheckErrorState()
  {
    bool ok = true;

    const rcutils_error_state_t * error_state;
    error_state = rcutils_get_error_state();

    ok &= error_state->file != NULL;
    ok &= error_state->line_number != 0;
    ok &= error_state->message != NULL;

    // if (ok) std::cout << error_state->file << ":"
    //  << error_state->line_number << " -> " << error_state->message << std::endl;

    return ok;
  }

  void TearDown()
  {
    // Stop agent
    server->stop();
  }

  rmw_node_t * node;
  std::unique_ptr<eprosima::uxr::Server> server;

  char topic_name[14] = "topic_name_01";
  char sub_name[12] = "sub_name_01";
};

/*
   Testing subscription construction and destruction.
 */
TEST_F(TestSubscription, construction_and_destruction) {
  rosidl_message_type_support_t dummy_type_support;
  message_type_support_callbacks_t dummy_callbacks;
  ConfigureDummyTypeSupport(&dummy_type_support, &dummy_callbacks);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  topic_name[sizeof(topic_name) - 2]++;
  sub_name[sizeof(sub_name) - 2]++;
  rmw_subscription_t * sub = rmw_create_subscription(this->node, &dummy_type_support,
      sub_name, &dummy_qos_policies,
      ignore_local_publications);
  ASSERT_NE((void *)sub, (void *)NULL);

  rmw_ret_t ret = rmw_destroy_subscription(this->node, sub);
  ASSERT_EQ(ret, RMW_RET_OK);
}


/*
   Testing node memory poll
 */
TEST_F(TestSubscription, memory_poll) {
  rosidl_message_type_support_t dummy_type_support;
  message_type_support_callbacks_t dummy_callbacks;
  ConfigureDummyTypeSupport(&dummy_type_support, &dummy_callbacks);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  std::vector<rmw_subscription_t *> subscriptions;
  rmw_ret_t ret;
  rmw_subscription_t * subscription;


  // Get all available nodes
  for (size_t i = 0; i < MAX_SUBSCRIPTIONS_X_NODE; i++) {
    topic_name[sizeof(topic_name) - 2]++;
    sub_name[sizeof(sub_name) - 2]++;
    subscription = rmw_create_subscription(this->node, &dummy_type_support,
        sub_name, &dummy_qos_policies,
        ignore_local_publications);
    ASSERT_NE((void *)subscription, (void *)NULL);
    subscriptions.push_back(subscription);
  }


  // Try to get one
  topic_name[sizeof(topic_name) - 2]++;
  sub_name[sizeof(sub_name) - 2]++;
  subscription = rmw_create_subscription(this->node, &dummy_type_support,
      sub_name, &dummy_qos_policies,
      ignore_local_publications);
  ASSERT_EQ((void *)subscription, (void *)NULL);
  ASSERT_EQ(CheckErrorState(), true);


  // Relese one
  subscription = subscriptions.back();
  subscriptions.pop_back();
  ret = rmw_destroy_subscription(this->node, subscription);
  ASSERT_EQ(ret, RMW_RET_OK);


  // Get one
  topic_name[sizeof(topic_name) - 2]++;
  sub_name[sizeof(sub_name) - 2]++;
  subscription = rmw_create_subscription(this->node, &dummy_type_support,
      sub_name, &dummy_qos_policies,
      ignore_local_publications);
  ASSERT_NE((void *)subscription, (void *)NULL);
  subscriptions.push_back(subscription);


  // Release all
  for (size_t i = 0; i < subscriptions.size(); i++) {
    subscription = subscriptions.at(i);
    ret = rmw_destroy_subscription(this->node, subscription);
    ASSERT_EQ(ret, RMW_RET_OK);
  }
  subscriptions.clear();
}
