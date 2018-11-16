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

#include <rmw/error_handling.h>
#include <rmw/node_security_options.h>
#include <rmw/rmw.h>
#include <rmw/validate_namespace.h>
#include <rmw/validate_node_name.h>

#include <vector>
#include <memory>

#include "./config.h"

#include "./test_utils.hpp"

class TestNode : public ::testing::Test
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
  }
};

/*
   Testing node construction and destruction.
 */
TEST_F(TestNode, construction_and_destruction) {
  // Success creation
  {
    rmw_node_security_options_t security_options;
    rmw_node_t * node = rmw_create_node("my_node", "/ns", 0, &security_options);
    ASSERT_NE((void *)node, (void *)NULL);
    rmw_ret_t ret = rmw_destroy_node(node);
    ASSERT_EQ(ret, RMW_RET_OK);
  }


  // Unsuccess creation
  {
    rmw_node_security_options_t security_options;
    rmw_node_t * node = rmw_create_node("", "/ns", 0, &security_options);
    ASSERT_EQ((void *)node, (void *)NULL);
    ASSERT_EQ(CheckErrorState(), true);
  }

  // Unsuccess creation
  {
    rmw_node_security_options_t security_options;
    rmw_node_t * node = rmw_create_node("my_node", "", 0, &security_options);
    ASSERT_EQ((void *)node, (void *)NULL);
  }

  // Unsuccess creation
  {
    rmw_node_security_options_t security_options;
    rmw_node_t * node = rmw_create_node("my_node", "/ns", 0, NULL);
    ASSERT_EQ((void *)node, (void *)NULL);
    ASSERT_EQ(CheckErrorState(), true);
  }
}

/*
   Testing node memory poll
 */
TEST_F(TestNode, memory_poll) {
  rmw_node_security_options_t dummy_security_options;
  std::vector<rmw_node_t *> nodes;
  rmw_ret_t ret;
  rmw_node_t * node;

  // Get all available nodes
  for (size_t i = 0; i < MAX_NODES; i++) {
    node = rmw_create_node("my_node", "/ns", 0, &dummy_security_options);
    ASSERT_NE((void *)node, (void *)NULL);
    nodes.push_back(node);
  }

  // Try to get one
  node = rmw_create_node("my_node", "/ns", 0, &dummy_security_options);
  ASSERT_EQ((void *)node, (void *)NULL);
  ASSERT_EQ(CheckErrorState(), true);

  // Relese one
  ret = rmw_destroy_node(nodes.back());
  ASSERT_EQ(ret, RMW_RET_OK);
  nodes.pop_back();

  // Get one
  node = rmw_create_node("my_node", "/ns", 0, &dummy_security_options);
  ASSERT_NE((void *)node, (void *)NULL);
  nodes.push_back(node);

  // Release all
  for (size_t i = 0; i < nodes.size(); i++) {
    ret = rmw_destroy_node(nodes.at(i));
    ASSERT_EQ(ret, RMW_RET_OK);
  }
  nodes.clear();
}
