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
#include "./rmw_microxrcedds_topic.h"

#include "./test_utils.hpp"


class TestTopic : public ::testing::Test
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

  void TearDown()
  {
    // Stop agent
    server->stop();
  }

  rmw_node_t * node;
  std::unique_ptr<eprosima::uxr::Server> server;
  const size_t attempts = 10;

  const char * topic_type = "topic_type";
  const char * topic_name = "topic_name";
  const char * package_name = "package_name";
};


/*
   Testing topic construction and destruction.
 */
TEST_F(TestTopic, construction_and_destruction) {
  rosidl_message_type_support_t dummy_type_support;
  message_type_support_callbacks_t dummy_callbacks;
  ConfigureDummyTypeSupport(
    topic_type,
    package_name,
    &dummy_type_support,
    &dummy_callbacks);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  custom_topic_t * topic = create_topic(
    reinterpret_cast<struct CustomNode *>(node->data),
    package_name,
    &dummy_callbacks,
    &dummy_qos_policies);
  ASSERT_NE((void *)topic, (void *)NULL);
  ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), 1);

  bool ret = destroy_topic(topic);
  ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), 0);
  ASSERT_EQ(ret, true);
}


/*
   Testing shared creation of a topic
 */
TEST_F(TestTopic, shared_topic_creation) {
  rosidl_message_type_support_t dummy_type_support;
  message_type_support_callbacks_t dummy_callbacks;
  ConfigureDummyTypeSupport(
    topic_type,
    package_name,
    &dummy_type_support,
    &dummy_callbacks);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  custom_topic_t * created_topic;
  custom_topic_t * last_created_topic;
  for (size_t i = 0; i < attempts; i++) {
    created_topic = create_topic(
      reinterpret_cast<struct CustomNode *>(node->data),
      topic_name,
      &dummy_callbacks,
      &dummy_qos_policies);

    if (i != 0) {
      ASSERT_EQ((void *)last_created_topic, (void *)created_topic);
    } else {
      ASSERT_NE((void *)created_topic, (void *)NULL);
    }
    ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), 1);

    last_created_topic = created_topic;
  }

  for (size_t i = 0; i < attempts; i++) {
    ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), 1);
    bool ret = destroy_topic(created_topic);
    ASSERT_EQ(ret, true);
  }
  ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), 0);
}


/*
   Testing creation multiple topics
 */
TEST_F(TestTopic, multiple_topic_creation) {
  rosidl_message_type_support_t dummy_type_support;
  message_type_support_callbacks_t dummy_callbacks;
  ConfigureDummyTypeSupport(
    topic_type,
    package_name,
    &dummy_type_support,
    &dummy_callbacks);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  std::vector<custom_topic_t *> created_topics;
  for (size_t i = 0; i < attempts; i++) {
    std::string aux_string(topic_type);
    aux_string.append(std::to_string(i));
    dummy_callbacks.message_name_ = aux_string.data();

    custom_topic_t * created_topic = create_topic(
      reinterpret_cast<struct CustomNode *>(node->data),
      std::string(topic_name).append(std::to_string(i)).data(),
      &dummy_callbacks,
      &dummy_qos_policies);

    ASSERT_NE((void *)created_topic, (void *)NULL);
    ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), i + 1);

    created_topics.push_back(created_topic);
  }

  for (size_t i = 0; i < created_topics.size(); i++) {
    ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), attempts - i);
    bool ret = destroy_topic(created_topics.at(i));
    ASSERT_EQ(ret, true);
  }
  ASSERT_EQ(topic_count(reinterpret_cast<struct CustomNode *>(node->data)), 0);
}
