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

#include <memory>
#include <string>

#include <chrono>
#include <thread>

#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "./rmw_base_test.hpp"
#include "./test_utils.hpp"

#include "rosidl_runtime_c/string.h"

#define MICROXRCEDDS_PADDING    sizeof(uint32_t)


class TestPubSub : public RMWBaseTest
{
protected:
    size_t id_gen = 0;

    const char* topic_type        = "topic_type";
    const char* topic_name        = "topic_name";
    const char* message_namespace = "package_name";
};

/*
 * Testing publish and subcribe to the same topic in diferent nodes
 */
TEST_F(TestPubSub, publish_and_receive)
{
    dummy_type_support_t dummy_type_support;

    ConfigureDummyTypeSupport(
        topic_type,
        topic_type,
        message_namespace,
        id_gen++,
        &dummy_type_support);

    dummy_type_support.callbacks.cdr_serialize =
        [](const void* untyped_ros_message, ucdrBuffer* cdr) -> bool {
            bool ret;
            const rosidl_runtime_c__String* ros_message =
                reinterpret_cast <const rosidl_runtime_c__String*>(untyped_ros_message);

            ret = ucdr_serialize_string(cdr, ros_message->data);
            return(ret);
        };
    dummy_type_support.callbacks.cdr_deserialize =
        [](ucdrBuffer* cdr, void* untyped_ros_message) -> bool {
            bool ret;
            rosidl_runtime_c__String* ros_message =
                reinterpret_cast <rosidl_runtime_c__String*>(untyped_ros_message);

            ret = ucdr_deserialize_string(cdr, ros_message->data, ros_message->capacity);
            if (ret)
            {
                ros_message->size = strlen(ros_message->data);
            }
            return(ret);
        };
    dummy_type_support.callbacks.get_serialized_size =
        [](const void* untyped_ros_message) -> uint32_t {
            const rosidl_runtime_c__String* ros_message =
                reinterpret_cast <const rosidl_runtime_c__String*>(untyped_ros_message);

            return(MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + ros_message->size + 8);
        };
    dummy_type_support.callbacks.max_serialized_size = []() -> size_t {
                                                           return((size_t)(MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + 1));
                                                       };

    rmw_qos_profile_t dummy_qos_policies;
    ConfigureDefaultQOSPolices(&dummy_qos_policies);

    rmw_node_t* node_pub;
    node_pub = rmw_create_node(&test_context, "pub_node", "/ns");
    ASSERT_NE((void*)node_pub, (void*)NULL);

    rmw_publisher_options_t default_publisher_options = rmw_get_default_publisher_options();
    rmw_publisher_t*        pub = rmw_create_publisher(
        node_pub, &dummy_type_support.type_support,
        topic_name, &dummy_qos_policies, &default_publisher_options);
    ASSERT_NE((void*)pub, (void*)NULL);

    rmw_node_t* node_sub;
    node_sub = rmw_create_node(&test_context, "sub_node", "/ns");
    ASSERT_NE((void*)node_sub, (void*)NULL);

    rmw_subscription_options_t default_subscription_options = rmw_get_default_subscription_options();

    rmw_subscription_t* sub = rmw_create_subscription(
        node_sub, &dummy_type_support.type_support,
        topic_name, &dummy_qos_policies, &default_subscription_options);
    ASSERT_NE((void*)sub, (void*)NULL);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    char content[] = "Test message XXXX";
    rosidl_runtime_c__String ros_message;
    ros_message.data     = content;
    ros_message.capacity = strlen(ros_message.data);
    ros_message.size     = ros_message.capacity;

    ASSERT_EQ(rmw_publish(pub, &ros_message, NULL), RMW_RET_OK);

    rmw_subscriptions_t subscriptions;
    subscriptions.subscribers      = &sub->data;
    subscriptions.subscriber_count = 1;

    rmw_guard_conditions_t guard_conditions;
    guard_conditions.guard_condition_count = 0;

    rmw_services_t services;
    services.service_count = 0;

    rmw_clients_t clients;
    clients.client_count = 0;

    rmw_events_t*   events   = NULL;
    rmw_wait_set_t* wait_set = NULL;
    rmw_time_t      wait_timeout;

    wait_timeout.sec  = 1;
    wait_timeout.nsec = 1;

    ASSERT_EQ(
        rmw_wait(
            &subscriptions,
            &guard_conditions,
            &services,
            &clients,
            events,
            wait_set,
            &wait_timeout
            ), RMW_RET_OK);

    ASSERT_NE((void*)subscriptions.subscribers[0], (void*)NULL);

    rosidl_runtime_c__String read_ros_message;
    char buff[100];
    read_ros_message.data     = buff;
    read_ros_message.capacity = sizeof(buff);
    read_ros_message.size     = 0;

    bool taken = false;
    ASSERT_EQ(
        rmw_take_with_info(
            sub,
            &read_ros_message,
            &taken,
            NULL,
            NULL
            ), RMW_RET_OK);

    ASSERT_EQ(taken, true);
    ASSERT_EQ(strcmp(content, read_ros_message.data), 0);
    ASSERT_EQ(strcmp(ros_message.data, read_ros_message.data), 0);
    ASSERT_EQ(ros_message.size, read_ros_message.size);
}
