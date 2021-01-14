// Copyright 2018-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "rosidl_typesupport_microxrcedds_c/message_type_support.h"

#include <vector>
#include <memory>
#include <string>

#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "rmw_microxrcedds_c/config.h"
#include "rmw_base_test.hpp"
#include "test_utils.hpp"

class TestPublisher : public RMWBaseTest
{
protected:
  void SetUp() override
  {
    RMWBaseTest::SetUp();

    node = rmw_create_node(&test_context, "my_node", "/ns");
    ASSERT_NE((void *)node, (void *)NULL);
  }

  void TearDown() override
  {
    ASSERT_EQ(rmw_destroy_node(node), RMW_RET_OK);
    RMWBaseTest::TearDown();
  }

  rmw_node_t * node;

  const char * topic_type = "topic_type";
  const char * topic_name = "topic_name";
  const char * message_namespace = "package_name";

  size_t id_gen = 0;
};

/*
   Testing subscription construction and destruction.
 */
TEST_F(TestPublisher, construction_and_destruction) {
  dummy_type_support_t dummy_type_support;
  ConfigureDummyTypeSupport(
    topic_type,
    topic_type,
    message_namespace,
    id_gen++,
    &dummy_type_support);


  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  rmw_publisher_options_t default_publisher_options = rmw_get_default_publisher_options();

  bool ignore_local_publications = true;

  rmw_publisher_t * pub = rmw_create_publisher(
    this->node,
    &dummy_type_support.type_support,
    topic_name,
    &dummy_qos_policies,
    &default_publisher_options);
  ASSERT_NE((void *)pub, (void *)NULL);

  rmw_ret_t ret = rmw_destroy_publisher(this->node, pub);
  ASSERT_EQ(ret, RMW_RET_OK);
}


/*
   Testing node memory poll for diferent topic
 */
TEST_F(TestPublisher, memory_poll_multiple_topic) {
  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  std::vector<dummy_type_support_t> dummy_type_supports;
  std::vector<rmw_publisher_t *> publishers;
  rmw_ret_t ret;
  rmw_publisher_t * publisher;

  rmw_publisher_options_t default_publisher_options = rmw_get_default_publisher_options();

  // Get all available nodes
  {
    for (size_t i = 0; i < RMW_UXRCE_MAX_PUBLISHERS; i++) {
      dummy_type_supports.push_back(dummy_type_support_t());
      ConfigureDummyTypeSupport(
        topic_type,
        topic_type,
        message_namespace,
        id_gen++,
        &dummy_type_supports.back());


      publisher = rmw_create_publisher(
        this->node,
        &dummy_type_supports.back().type_support,
        dummy_type_supports.back().topic_name.data(),
        &dummy_qos_policies,
        &default_publisher_options);
      ASSERT_NE((void *)publisher, (void *)NULL);
      publishers.push_back(publisher);
    }
  }


  // Try to get one
  {
    dummy_type_supports.push_back(dummy_type_support_t());
    ConfigureDummyTypeSupport(
      topic_type,
      topic_type,
      message_namespace,
      id_gen++,
      &dummy_type_supports.back());

    publisher = rmw_create_publisher(
      this->node,
      &dummy_type_supports.back().type_support,
      dummy_type_supports.back().topic_name.data(),
      &dummy_qos_policies,
      &default_publisher_options);
    ASSERT_EQ((void *)publisher, (void *)NULL);
    ASSERT_EQ(CheckErrorState(), true);

    // Relese one
    publisher = publishers.back();
    publishers.pop_back();
    ret = rmw_destroy_publisher(this->node, publisher);
    ASSERT_EQ(ret, RMW_RET_OK);
  }


  // Get one
  {
    dummy_type_supports.push_back(dummy_type_support_t());
    ConfigureDummyTypeSupport(
      topic_type,
      topic_type,
      message_namespace,
      id_gen++,
      &dummy_type_supports.back());

    publisher = rmw_create_publisher(
      this->node,
      &dummy_type_supports.back().type_support,
      dummy_type_supports.back().topic_name.data(),
      &dummy_qos_policies,
      &default_publisher_options);
    ASSERT_NE((void *)publisher, (void *)NULL);
    publishers.push_back(publisher);
  }


  // Release all
  {
    for (size_t i = 0; i < publishers.size(); i++) {
      publisher = publishers.at(i);
      ret = rmw_destroy_publisher(this->node, publisher);
      ASSERT_EQ(ret, RMW_RET_OK);
    }
    publishers.clear();
  }
}


/*
   Testing node memory poll for same topic
 */
TEST_F(TestPublisher, memory_poll_shared_topic) {
  dummy_type_support_t dummy_type_support;
  ConfigureDummyTypeSupport(
    topic_type,
    topic_type,
    message_namespace,
    id_gen++,
    &dummy_type_support);

  rmw_qos_profile_t dummy_qos_policies;
  ConfigureDefaultQOSPolices(&dummy_qos_policies);

  bool ignore_local_publications = true;

  std::vector<rmw_publisher_t *> publishers;
  rmw_ret_t ret;
  rmw_publisher_t * publisher;

  rmw_publisher_options_t default_publisher_options = rmw_get_default_publisher_options();

  // Get all available nodes
  {
    for (size_t i = 0; i < RMW_UXRCE_MAX_PUBLISHERS; i++) {
      publisher = rmw_create_publisher(
        this->node,
        &dummy_type_support.type_support,
        topic_name,
        &dummy_qos_policies,
        &default_publisher_options);
      ASSERT_NE((void *)publisher, (void *)NULL);
      publishers.push_back(publisher);
    }
  }


  // Try to get one
  {
    publisher = rmw_create_publisher(
      this->node,
      &dummy_type_support.type_support,
      topic_name,
      &dummy_qos_policies,
      &default_publisher_options);
    ASSERT_EQ((void *)publisher, (void *)NULL);
    ASSERT_EQ(CheckErrorState(), true);

    // Relese one
    publisher = publishers.back();
    publishers.pop_back();
    ret = rmw_destroy_publisher(this->node, publisher);
    ASSERT_EQ(ret, RMW_RET_OK);
  }


  // Get one
  {
    publisher = rmw_create_publisher(
      this->node,
      &dummy_type_support.type_support,
      topic_name,
      &dummy_qos_policies,
      &default_publisher_options);
    ASSERT_NE((void *)publisher, (void *)NULL);
    publishers.push_back(publisher);
  }


  // Release all
  {
    for (size_t i = 0; i < publishers.size(); i++) {
      publisher = publishers.at(i);
      ret = rmw_destroy_publisher(this->node, publisher);
      ASSERT_EQ(ret, RMW_RET_OK);
    }

  // Destroy an already detroyed publisher
  ret = rmw_destroy_publisher(this->node, publishers.at(0));
  ASSERT_EQ(ret, RMW_RET_ERROR);

    publishers.clear();
  }
}
