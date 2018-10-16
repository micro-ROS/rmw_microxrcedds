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


#include "rmw/error_handling.h"
#include "rmw/node_security_options.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#define EXPECT_NULL(ptr) EXPECT_EQ((void *)ptr, (void *)NULL)
#define EXPECT_NON_NULL(ptr) EXPECT_NE((void *)ptr, (void *)NULL)

class TestNode : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    rmw_init();
  }
};

/*
   Testing node construction and destruction.
 */
TEST_F(TestNode, construction_and_destruction) 
{
  rmw_node_security_options_t security_options;

  {
    
    rmw_node_t * node = rmw_create_node("my_node", "/ns", 0, &security_options);
    EXPECT_NON_NULL(node);
    rmw_destroy_node(node);
  }

  {
    rmw_node_t * node = rmw_create_node("", "/ns", 0, &security_options);
    EXPECT_NULL(node);
  }


  {
    rmw_node_t * node = rmw_create_node("my_node", "", 0, &security_options);
    EXPECT_NULL(node);
  }

  {
    rmw_node_t * node = rmw_create_node("my_node", "/ns", 0, NULL);
    EXPECT_NULL(node);
  }

}
