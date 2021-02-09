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

#include <uxr/client/core/session/session_info.h>

#define MICROXRCEDDS_PADDING    sizeof(uint32_t)

dummy_service_type_support_t dummy_type_support;

const rosidl_message_type_support_t* get_request_members()
{
    return(&dummy_type_support.request_members.type_support);
};

const rosidl_message_type_support_t* get_response_members()
{
    return(&dummy_type_support.response_members.type_support);
};

class TestReqRes : public RMWBaseTest
{
protected:
    size_t id_gen = 0;


    const char* service_type      = "topic_type";
    const char* service_name      = "topic_name";
    const char* message_namespace = "package_name";
};

/*
 * Testing request and reply
 */
TEST_F(TestReqRes, publish_and_receive)
{
    // CREATE FAKE TYPESUPPORT

    ConfigureDummyServiceTypeSupport(
        service_type,
        service_name,
        "",
        id_gen++,
        &dummy_type_support);

    dummy_type_support.callbacks.response_members_ = get_request_members;
    dummy_type_support.callbacks.request_members_  = get_response_members;

    dummy_type_support.request_members.callbacks.cdr_serialize =
        [](const void* untyped_service_req_message, ucdrBuffer* cdr) -> bool {
            bool ret;
            const rosidl_runtime_c__String* service_req_message =
                reinterpret_cast <const rosidl_runtime_c__String*>(untyped_service_req_message);

            ret = ucdr_serialize_string(cdr, service_req_message->data);
            return(ret);
        };
    dummy_type_support.request_members.callbacks.cdr_deserialize =
        [](ucdrBuffer* cdr, void* untyped_service_req_message) -> bool {
            bool ret;
            rosidl_runtime_c__String* service_req_message =
                reinterpret_cast <rosidl_runtime_c__String*>(untyped_service_req_message);

            ret = ucdr_deserialize_string(cdr, service_req_message->data, service_req_message->capacity);
            if (ret)
            {
                service_req_message->size = strlen(service_req_message->data);
            }
            return(ret);
        };
    dummy_type_support.request_members.callbacks.get_serialized_size =
        [](const void* untyped_service_req_message) -> uint32_t {
            const rosidl_runtime_c__String* service_req_message =
                reinterpret_cast <const rosidl_runtime_c__String*>(untyped_service_req_message);

            return(MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + service_req_message->size + 8);
        };
    dummy_type_support.request_members.callbacks.max_serialized_size = []() -> size_t {
                                                                           return((size_t)(MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + 1));
                                                                       };

    dummy_type_support.response_members.callbacks.cdr_serialize       = dummy_type_support.request_members.callbacks.cdr_serialize;
    dummy_type_support.response_members.callbacks.cdr_deserialize     = dummy_type_support.request_members.callbacks.cdr_deserialize;
    dummy_type_support.response_members.callbacks.get_serialized_size = dummy_type_support.request_members.callbacks.get_serialized_size;
    dummy_type_support.response_members.callbacks.max_serialized_size = dummy_type_support.request_members.callbacks.max_serialized_size;

    rmw_qos_profile_t dummy_qos_policies;
    ConfigureDefaultQOSPolices(&dummy_qos_policies);

    // CREATE ENTITIES

    rmw_node_t* node_res;
    node_res = rmw_create_node(&test_context, "node_res", "/ns");
    ASSERT_NE((void*)node_res, (void*)NULL);

    rmw_service_t* serv = rmw_create_service(
        node_res,
        &dummy_type_support.type_support,
        service_name,
        &dummy_qos_policies);

    ASSERT_NE((void*)serv, (void*)NULL);

    rmw_node_t* node_req;
    node_req = rmw_create_node(&test_context, "node_req", "/ns");
    ASSERT_NE((void*)node_req, (void*)NULL);

    rmw_client_t* client = rmw_create_client(
        node_req,
        &dummy_type_support.type_support,
        service_name,
        &dummy_qos_policies);

    ASSERT_NE((void*)client, (void*)NULL);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // SEND REQUEST

    char content_req[] = "SERVICE_REQUEST";
    rosidl_runtime_c__String service_req_message;
    service_req_message.data     = content_req;
    service_req_message.capacity = strlen(service_req_message.data);
    service_req_message.size     = service_req_message.capacity;

    int64_t sequence_id;
    ASSERT_EQ(rmw_send_request(client, &service_req_message, &sequence_id), RMW_RET_OK);
    ASSERT_NE(sequence_id, UXR_INVALID_REQUEST_ID);

    // RECEIVE REQUEST

    rmw_subscriptions_t subscriptions;
    subscriptions.subscriber_count = 0;

    rmw_guard_conditions_t guard_conditions;
    guard_conditions.guard_condition_count = 0;

    rmw_services_t services;
    services.services      = &serv->data;
    services.service_count = 1;

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

    ASSERT_NE((void*)services.services[0], (void*)NULL);

    rosidl_runtime_c__String read_service_req_message;
    char buff_req[100];
    read_service_req_message.data     = buff_req;
    read_service_req_message.capacity = sizeof(buff_req);
    read_service_req_message.size     = 0;

    bool taken = false;
    rmw_service_info_t request_header;

    ASSERT_EQ(rmw_take_request(
                  serv,
                  &request_header,
                  &read_service_req_message,
                  &taken
                  ), RMW_RET_OK);

    ASSERT_EQ(taken, true);
    ASSERT_EQ(strcmp(content_req, read_service_req_message.data), 0);
    ASSERT_EQ(strcmp(service_req_message.data, read_service_req_message.data), 0);
    ASSERT_EQ(service_req_message.size, read_service_req_message.size);

    // SEND RESPONSE

    char content_res[] = "SERVICE_RESPONSE";
    rosidl_runtime_c__String service_res_message;
    service_res_message.data     = content_res;
    service_res_message.capacity = strlen(service_res_message.data);
    service_res_message.size     = service_res_message.capacity;

    ASSERT_EQ(rmw_send_response(
                  serv,
                  &request_header.request_id,
                  &service_res_message
                  ), RMW_RET_OK);

    // READ REPONSE

    subscriptions.subscriber_count         = 0;
    guard_conditions.guard_condition_count = 0;
    services.service_count = 0;
    clients.clients        = &client->data;
    clients.client_count   = 1;

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

    ASSERT_NE((void*)clients.clients[0], (void*)NULL);

    rosidl_runtime_c__String read_service_res_message;
    char buff_res[100];
    read_service_res_message.data     = buff_res;
    read_service_res_message.capacity = sizeof(buff_res);
    read_service_res_message.size     = 0;
    read_service_res_message.size     = 0;
    taken = false;

    ASSERT_EQ(rmw_take_response(
                  client,
                  &request_header,
                  &read_service_res_message,
                  &taken
                  ), RMW_RET_OK);

    ASSERT_EQ(taken, true);
    ASSERT_EQ(request_header.request_id.sequence_number, sequence_id);
    ASSERT_EQ(strcmp(content_res, read_service_res_message.data), 0);
    ASSERT_EQ(strcmp(service_res_message.data, read_service_res_message.data), 0);
    ASSERT_EQ(service_res_message.size, read_service_res_message.size);
}
