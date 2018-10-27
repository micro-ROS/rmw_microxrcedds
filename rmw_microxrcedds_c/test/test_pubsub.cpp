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
#include <string>

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

#include "./test_utils.hpp"

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

  size_t id_gen = 0;

  const char * topic_type = "topic_type";
  const char * topic_name = "topic_name";
  const char * package_name = "package_name";
};

/*
   Testing publish and subcribe to the same topic in diferent nodes
 */
TEST_F(TestSubscription, publish_and_receive) {
  dummy_type_support_t dummy_type_support;
  ConfigureDummyTypeSupport(
    topic_type,
    topic_type,
    package_name,
    id_gen++,
    &dummy_type_support);

  dummy_type_support.callbacks.cdr_serialize =
    [](const void * untyped_ros_message, ucdrBuffer * cdr) -> bool {
      bool ok;
      ok = ucdr_serialize_string(cdr, reinterpret_cast<const char *>(untyped_ros_message));
      return ok;
    };
  dummy_type_support.callbacks.cdr_deserialize =
    [](ucdrBuffer * cdr, void * untyped_ros_message, uint8_t * raw_mem_ptr,
      size_t raw_mem_size) -> bool {
      uint32_t Aux_uint32;
      bool ok;

      ok = ucdr_deserialize_string(cdr, reinterpret_cast<char *>(raw_mem_ptr), raw_mem_size);
      *(reinterpret_cast<char **>(untyped_ros_message)) = reinterpret_cast<char *>(raw_mem_ptr);

      return ok;
    };
  dummy_type_support.callbacks.get_serialized_size = [](const void *) -> uint32_t {
      return MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + strlen(
        test_parameter) + 8;
    };
  dummy_type_support.callbacks.max_serialized_size = [](bool full_bounded) -> size_t {
      return (size_t)(MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + 1);
    };

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  rmw_node_security_options_t dummy_security_options;


  rmw_node_t * node_pub;
  node_pub = rmw_create_node("pub_node", "/ns", 0, &dummy_security_options);
  ASSERT_NE((void *)node_pub, (void *)NULL);


  rmw_publisher_t * pub = rmw_create_publisher(node_pub, &dummy_type_support.type_support,
      topic_name, &dummy_qos_policies);
  ASSERT_NE((void *)pub, (void *)NULL);

  rmw_node_t * node_sub;
  node_sub = rmw_create_node("sub_node", "/ns", 0, &dummy_security_options);
  ASSERT_NE((void *)node_sub, (void *)NULL);


  rmw_subscription_t * sub = rmw_create_subscription(node_sub, &dummy_type_support.type_support,
      topic_name, &dummy_qos_policies, ignore_local_publications);
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
