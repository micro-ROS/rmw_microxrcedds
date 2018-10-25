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

#include <memory>

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPServerWindows.hpp>
#else
#include <uxr/agent/transport/udp/UDPServerLinux.hpp>
#endif  // _WIN32

#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#include <rosidl_typesupport_microxrcedds_shared/identifier.h>
#include <rosidl_typesupport_microxrcedds_shared/message_type_support.h>

#include "rmw/error_handling.h"
#include "rmw/node_security_options.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#define MICROXRCEDDS_PADDING sizeof(uint32_t)

const char * test_parameter = "Test message XXXX";

class TestSubscription : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    GTEST_DECLARE_bool_(break_on_failure);

    #ifndef _WIN32
    freopen("/dev/null", "w", stderr);
    #endif

    rosidl_typesupport_microxrcedds_c__identifier =
      ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE;
  }

  void SetUp()
  {
    rmw_ret_t ret = rmw_init();
    ASSERT_EQ(ret, RMW_RET_OK);

    server =
      std::unique_ptr<eprosima::uxr::Server>(new eprosima::uxr::UDPServer((uint16_t)atoi("8888")));
    server->run();
    // ASSERT_EQ(server->run(), true);
  }

  void TearDown()
  {
    // Stop agent
    server->stop();
  }

  std::unique_ptr<eprosima::uxr::Server> server;
  rmw_ret_t ret;
};

/*
   Testing publish and subcribe to the same topic in diferent nodes
 */
TEST_F(TestSubscription, publish_and_receive) {
  message_type_support_callbacks_t dummy_callbacks;
  dummy_callbacks.message_name_ = "dummy";
  dummy_callbacks.package_name_ = "dummy";
  dummy_callbacks.cdr_serialize = [](const void * untyped_ros_message, ucdrBuffer * cdr) -> bool {
      bool ok;
      ok = ucdr_serialize_string(cdr, reinterpret_cast<const char *>(untyped_ros_message));
      return ok;
    };
  dummy_callbacks.cdr_deserialize =
    [](ucdrBuffer * cdr, void * untyped_ros_message, uint8_t * raw_mem_ptr,
      size_t raw_mem_size) -> bool {
      uint32_t Aux_uint32;
      bool ok;

      ok = ucdr_deserialize_string(cdr, reinterpret_cast<char *>(raw_mem_ptr), raw_mem_size);
      *(reinterpret_cast<char **>(untyped_ros_message)) = reinterpret_cast<char *>(raw_mem_ptr);

      return ok;
    };
  dummy_callbacks.get_serialized_size = [](const void *) -> uint32_t {
      return MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + strlen(
        test_parameter) + 8;
    };
  dummy_callbacks.max_serialized_size = [](bool full_bounded) -> size_t {
      return (size_t)(MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + 1);
    };


  rosidl_message_type_support_t dummy_type_support;
  dummy_type_support.typesupport_identifier = rosidl_typesupport_microxrcedds_c__identifier;
  dummy_type_support.data = &dummy_callbacks;
  dummy_type_support.func =
    [](const rosidl_message_type_support_t * type_support, const char * id) {
      return type_support;
    };

  rmw_qos_profile_t dummy_qos_policies;
  dummy_qos_policies.avoid_ros_namespace_conventions = false;
  dummy_qos_policies.depth = 0;
  dummy_qos_policies.durability = RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT;
  // dummy_qos_policies.durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
  // dummy_qos_policies.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
  dummy_qos_policies.history = RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT;
  // dummy_qos_policies.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  // dummy_qos_policies.history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;
  dummy_qos_policies.reliability = RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT;
  // dummy_qos_policies.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
  // dummy_qos_policies.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;

  bool ignore_local_publications = true;

  rmw_node_security_options_t dummy_security_options;


  rmw_node_t * node_pub;
  node_pub = rmw_create_node("pub_node", "/ns", 0, &dummy_security_options);
  ASSERT_NE((void *)node_pub, (void *)NULL);


  rmw_publisher_t * pub = rmw_create_publisher(node_pub, &dummy_type_support, "topic_name",
      &dummy_qos_policies);
  ASSERT_NE((void *)pub, (void *)NULL);

  rmw_node_t * node_sub;
  node_sub = rmw_create_node("sub_node", "/ns", 0, &dummy_security_options);
  ASSERT_NE((void *)node_sub, (void *)NULL);


  rmw_subscription_t * sub = rmw_create_subscription(node_sub, &dummy_type_support,
      "topic_name", &dummy_qos_policies,
      ignore_local_publications);
  ASSERT_NE((void *)sub, (void *)NULL);


  usleep(5000);

  ret = rmw_publish(pub, test_parameter);
  ASSERT_EQ(ret, RMW_RET_OK);


  rmw_subscriptions_t subscriptions;
  rmw_guard_conditions_t * guard_conditions = NULL;
  rmw_services_t * services = NULL;
  rmw_clients_t * clients = NULL;
  rmw_wait_set_t * wait_set = NULL;
  rmw_time_t wait_timeout;

  subscriptions.subscribers = reinterpret_cast<void **>(&sub->data);
  subscriptions.subscriber_count = 1;

  wait_timeout.sec = 1;

  ret = rmw_wait(
    &subscriptions,
    guard_conditions,
    services,
    clients,
    wait_set,
    &wait_timeout
    );
  ASSERT_EQ(ret, RMW_RET_OK);

  char * ReadMesg;
  bool taken;
  ret = rmw_take_with_info(
    sub,
    &ReadMesg,
    &taken,
    NULL
    );
  ASSERT_EQ(ret, RMW_RET_OK);
  ASSERT_EQ(taken, true);
  ASSERT_EQ(strcmp(test_parameter, ReadMesg), 0);
}
