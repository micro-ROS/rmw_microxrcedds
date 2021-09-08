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

#include <rmw/error_handling.h>
#include <rmw/rmw.h>
#include <rmw/validate_namespace.h>
#include <rmw/validate_node_name.h>

#include <rosidl_runtime_c/string.h>
#include <uxr/client/core/session/session_info.h>

#include <memory>
#include <string>
#include <chrono>
#include <thread>

#include "./rmw_base_test.hpp"
#include "./test_utils.hpp"

#define MICROXRCEDDS_PADDING    sizeof(uint32_t)


static dummy_service_type_support_t dummy_type_support;

class TestReqRes : public RMWBaseTest
{
public:
  void SetUp() override
  {
    RMWBaseTest::SetUp();

    configure_typesupport();

    node = rmw_create_node(&test_context, "node_reqres", "/ns");
    EXPECT_NE(node, nullptr);
  }

  void TearDown() override
  {
    for(auto srv: services){
      EXPECT_EQ(rmw_destroy_service(node, srv), RMW_RET_OK);
    }

    for(auto cli: clients){
      EXPECT_EQ(rmw_destroy_client(node, cli), RMW_RET_OK);
    }

    EXPECT_EQ(rmw_destroy_node(node), RMW_RET_OK);

    RMWBaseTest::TearDown();
  }

  void configure_typesupport()
  {
    ConfigureDummyServiceTypeSupport(
      service_type,
      service_name,
      "",
      id_gen++,
      &dummy_type_support);

    dummy_type_support.callbacks.response_members_ = []() -> const rosidl_message_type_support_t * {
        return &dummy_type_support.response_members.type_support;
      };
    dummy_type_support.callbacks.request_members_ = []() -> const rosidl_message_type_support_t * {
        return &dummy_type_support.request_members.type_support;
      };

    dummy_type_support.request_members.callbacks.cdr_serialize =
      [](const void * untyped_service_req_message, ucdrBuffer * cdr) -> bool
      {
        bool ret;
        const rosidl_runtime_c__String * service_req_message =
          reinterpret_cast<const rosidl_runtime_c__String *>(untyped_service_req_message);

        ret = ucdr_serialize_string(cdr, service_req_message->data);
        return ret;
      };

    dummy_type_support.request_members.callbacks.cdr_deserialize =
      [](ucdrBuffer * cdr, void * untyped_service_req_message) -> bool
      {
        bool ret;
        rosidl_runtime_c__String * service_req_message =
          reinterpret_cast<rosidl_runtime_c__String *>(untyped_service_req_message);

        ret =
          ucdr_deserialize_string(cdr, service_req_message->data, service_req_message->capacity);
        if (ret) {
          service_req_message->size = strlen(service_req_message->data);
        }
        return ret;
      };

    dummy_type_support.request_members.callbacks.get_serialized_size =
      [](const void * untyped_service_req_message) -> uint32_t
      {
        const rosidl_runtime_c__String * service_req_message =
          reinterpret_cast<const rosidl_runtime_c__String *>(untyped_service_req_message);

        return MICROXRCEDDS_PADDING +
               ucdr_alignment(0, MICROXRCEDDS_PADDING) + service_req_message->size + 8;
      };
    dummy_type_support.request_members.callbacks.max_serialized_size = []() -> size_t
      {
        return (size_t)(MICROXRCEDDS_PADDING + ucdr_alignment(0, MICROXRCEDDS_PADDING) + 1);
      };

    dummy_type_support.response_members.callbacks.cdr_serialize =
      dummy_type_support.request_members.callbacks.cdr_serialize;
    dummy_type_support.response_members.callbacks.cdr_deserialize =
      dummy_type_support.request_members.callbacks.cdr_deserialize;
    dummy_type_support.response_members.callbacks.get_serialized_size =
      dummy_type_support.request_members.callbacks.get_serialized_size;
    dummy_type_support.response_members.callbacks.max_serialized_size =
      dummy_type_support.request_members.callbacks.max_serialized_size;
  }

  rmw_service_t * create_service(rmw_qos_profile_t qos) {
    rmw_service_t * serv = rmw_create_service(node, &dummy_type_support.type_support, "test_service", &qos);
    EXPECT_NE(serv, nullptr);
    services.push_back(serv);
    return serv;
  }

  rmw_client_t * create_client(rmw_qos_profile_t qos) {
    rmw_client_t * client = rmw_create_client(node, &dummy_type_support.type_support, "test_service", &qos);
    EXPECT_NE(client, nullptr);
    clients.push_back(client);
    return client;
  }

  void send_request(const char * data, rmw_client_t * client, int64_t & sequence_id) {
    rosidl_runtime_c__String service_req_message;
    service_req_message.data = const_cast<char *>(data);;
    service_req_message.capacity = strlen(data);
    service_req_message.size = service_req_message.capacity;

    ASSERT_EQ(rmw_send_request(client, &service_req_message, &sequence_id), RMW_RET_OK);
    ASSERT_NE(sequence_id, UXR_INVALID_REQUEST_ID);
  }

  void send_response(const char * data, rmw_service_t * serv, rmw_service_info_t & request_header) {
    rosidl_runtime_c__String service_req_message;
    service_req_message.data = const_cast<char *>(data);;
    service_req_message.capacity = strlen(data);
    service_req_message.size = service_req_message.capacity;

    ASSERT_EQ(rmw_send_response(serv, &request_header.request_id, &service_req_message), RMW_RET_OK);
  }

  rmw_ret_t wait_for_request(rmw_service_t * serv){
    rmw_services_t services = {};
    void * servs[1] = {serv->data};
    services.services = servs;
    services.service_count = 1;
    rmw_subscriptions_t subscriptions = {};
    rmw_guard_conditions_t guard_conditions = {};
    rmw_clients_t clients = {};
    rmw_events_t * events = NULL;
    rmw_wait_set_t * wait_set = NULL;

    rmw_time_t wait_timeout = (rmw_time_t) {1LL, 1LL};

    return rmw_wait(
      &subscriptions, &guard_conditions,
      &services, &clients, events, wait_set,
      &wait_timeout);
  }

  rmw_ret_t wait_for_response(rmw_client_t * client){
    rmw_clients_t clients = {};
    void * clis[1] = {client->data};
    clients.clients = clis;
    clients.client_count = 1;
    rmw_services_t services = {};
    rmw_subscriptions_t subscriptions = {};
    rmw_guard_conditions_t guard_conditions = {};
    rmw_events_t * events = NULL;
    rmw_wait_set_t * wait_set = NULL;

    rmw_time_t wait_timeout = (rmw_time_t) {1LL, 1LL};

    return rmw_wait(
      &subscriptions, &guard_conditions,
      &services, &clients, events, wait_set,
      &wait_timeout);
  }

  rmw_ret_t take_request(
    rmw_service_t * serv, char * buff, size_t buff_size,
    bool & taken, rmw_service_info_t & request_header)
  {
    rosidl_runtime_c__String read_ros_message;
    read_ros_message.data = buff;
    read_ros_message.capacity = buff_size;
    read_ros_message.size = 0;

    return rmw_take_request(serv, &request_header, &read_ros_message, &taken);
  }

  rmw_ret_t take_response(
    rmw_client_t * client, char * buff, size_t buff_size,
    bool & taken, rmw_service_info_t & request_header)
  {
    rosidl_runtime_c__String read_ros_message;
    read_ros_message.data = buff;
    read_ros_message.capacity = buff_size;
    read_ros_message.size = 0;

    return rmw_take_response(client, &request_header, &read_ros_message, &taken);
  }

protected:
  size_t id_gen = 0;

  const char * service_type = "topic_type";
  const char * service_name = "topic_name";
  const char * message_namespace = "package_name";

  rmw_node_t * node;

  std::vector<rmw_client_t *> clients;
  std::vector<rmw_service_t *> services;
};

/*
 * Testing request and reply
 */
TEST_F(TestReqRes, request_and_reply)
{
  rmw_service_t * service = create_service(rmw_qos_profile_services_default);
  rmw_client_t * client = create_client(rmw_qos_profile_services_default);

  // Request
  std::string req_data = "test_request";
  int64_t sequence_id = -1;
  send_request(req_data.c_str(), client, sequence_id);

  ASSERT_EQ(wait_for_request(service), RMW_RET_OK);

  bool taken = false;
  rmw_service_info_t request_header;
  char req_recv_data[100] = {0};
  ASSERT_EQ(take_request(service, req_recv_data, sizeof(req_recv_data), taken, request_header), RMW_RET_OK);

  ASSERT_EQ(taken, true);
  ASSERT_EQ(strcmp(req_data.c_str(), req_recv_data), 0);

  // Response
  std::string res_data = "test_response";
  send_response(res_data.c_str(), service, request_header);

  ASSERT_EQ(wait_for_response(client), RMW_RET_OK);

  taken = false;
  rmw_service_info_t response_header;
  char res_recv_data[100] = {0};
  ASSERT_EQ(take_response(client, res_recv_data, sizeof(res_recv_data), taken, response_header), RMW_RET_OK);

  ASSERT_EQ(taken, true);
  ASSERT_EQ(sequence_id, response_header.request_id.sequence_number);
  ASSERT_EQ(strcmp(res_data.c_str(), res_recv_data), 0);
}
