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

#include <vector>
#include <memory>
#include <string>

#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include <rmw_microxrcedds_c/config.h>
#include "rmw_microxrcedds_topic.h"
#include "rmw_base_test.hpp"
#include "test_utils.hpp"

class TestTopic : public RMWBaseTest
{
protected:
  void SetUp() override
  {
    RMWBaseTest::SetUp();

    node = rmw_create_node(&test_context, "my_node", "/ns", 0, false);
    EXPECT_NE((void *)node, (void *)NULL);
  }

  void TearDown() override
  {
    EXPECT_EQ(rmw_destroy_node(node), RMW_RET_OK);
    RMWBaseTest::TearDown();
  }

  rmw_node_t * node;
  const size_t attempts = 10;
  size_t id_gen = 0;

  const char * topic_type = "topic_type";
  const char * topic_name = "topic_name";
  const char * package_name = "package_name";
};


/*
   Testing topic construction and destruction.
 */
TEST_F(TestTopic, construction_and_destruction) {
  dummy_type_support_t dummy_type_support;
  ConfigureDummyTypeSupport(
    topic_type,
    topic_type,
    package_name,
    id_gen++,
    &dummy_type_support);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  rmw_uxrce_topic_t * topic = create_topic(
    reinterpret_cast<struct rmw_uxrce_node_t *>(node->data),
    package_name,
    &dummy_type_support.callbacks,
    &dummy_qos_policies);
  EXPECT_NE((void *)topic, (void *)NULL);
  EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), 1);

  bool ret = destroy_topic(topic);
  EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), 0);
  EXPECT_EQ(ret, true);
}


/*
   Testing shared creation of a topic
 */
TEST_F(TestTopic, shared_topic_creation) {
  dummy_type_support_t dummy_type_support;
  ConfigureDummyTypeSupport(
    topic_type,
    topic_type,
    package_name,
    id_gen++,
    &dummy_type_support);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  rmw_uxrce_topic_t * created_topic;
  rmw_uxrce_topic_t * last_created_topic;
  for (size_t i = 0; i < attempts; i++) {
    created_topic = create_topic(
      reinterpret_cast<struct rmw_uxrce_node_t *>(node->data),
      topic_name,
      &dummy_type_support.callbacks,
      &dummy_qos_policies);

    if (i != 0) {
      EXPECT_EQ((void *)last_created_topic, (void *)created_topic);
    } else {
      EXPECT_NE((void *)created_topic, (void *)NULL);
    }
    EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), 1);

    last_created_topic = created_topic;
  }

  for (size_t i = 0; i < attempts; i++) {
    EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), 1);
    bool ret = destroy_topic(created_topic);
    EXPECT_EQ(ret, true);
  }
  EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), 0);
}


/*
   Testing creation multiple topics
 */
TEST_F(TestTopic, multiple_topic_creation) {
  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  std::vector<rmw_uxrce_topic_t *> created_topics;
  std::vector<dummy_type_support_t> dummy_type_supports;
  for (size_t i = 0; i < attempts; i++) {
    dummy_type_supports.push_back(dummy_type_support_t());
    ConfigureDummyTypeSupport(
      topic_type,
      topic_type,
      package_name,
      id_gen++,
      &dummy_type_supports.back());

    rmw_uxrce_topic_t * created_topic = create_topic(
      reinterpret_cast<struct rmw_uxrce_node_t *>(node->data),
      dummy_type_supports.back().topic_name.data(),
      &dummy_type_supports.back().callbacks,
      &dummy_qos_policies);

    EXPECT_NE((void *)created_topic, (void *)NULL);
    EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), i + 1);

    created_topics.push_back(created_topic);
  }

  for (size_t i = 0; i < created_topics.size(); i++) {
    EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), attempts - i);
    bool ret = destroy_topic(created_topics.at(i));
    EXPECT_EQ(ret, true);
  }
  EXPECT_EQ(topic_count(reinterpret_cast<struct rmw_uxrce_node_t *>(node->data)), 0);
}
