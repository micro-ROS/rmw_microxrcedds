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


class TestSubscription : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    rmw_ret_t ret = rmw_init();
    EXPECT_EQ(ret,RMW_RET_OK);
  }

  void SetUp()
  {
    rmw_node_security_options_t security_options;
    node = rmw_create_node("my_node", "/ns", 0, &security_options);
    EXPECT_NE((void*)node, (void*)NULL);
  }

  void TearDown()
  {
    rmw_ret_t ret = rmw_destroy_node(node);
    EXPECT_EQ(ret,RMW_RET_OK);
  }

  rmw_node_t * node;
  
};

/*
   Testing subscription construction and destruction.
 */
TEST_F(TestSubscription, construction_and_destruction) {
    
    // Test 
    {
        rosidl_message_type_support_t dummy_type_support;
        dummy_type_support.typesupport_identifier = "";
        dummy_type_support.data = NULL;
        dummy_type_support.func = NULL;

        rmw_qos_profile_t dummy_qos_policies;
        dummy_qos_policies.avoid_ros_namespace_conventions = false;
        
        dummy_qos_policies.depth = 0;
        
        dummy_qos_policies.durability = RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT;
        //dummy_qos_policies.durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
        //dummy_qos_policies.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
        
        dummy_qos_policies.history = RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT;
        //dummy_qos_policies.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
        //dummy_qos_policies.history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;
        
        dummy_qos_policies.reliability = RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT;
        //dummy_qos_policies.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
        //dummy_qos_policies.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;




        rmw_publisher_t * pub = rmw_create_publisher(this->node, &dummy_type_support, "topic_name", &dummy_qos_policies);

        //EXPECT_NON_NULL(pub);
        rmw_ret_t ret = rmw_destroy_publisher(this->node, pub);
        EXPECT_EQ(ret,RMW_RET_OK);
    }

  // {
  //   rclc_subscription_t * sub =
  //     rclc_create_subscription(node,
  //       RCLC_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, IntraProcessMessage),
  //       "invalid_topic?", callback, 0, false);
  //   EXPECT_NULL(sub);
  // }
}