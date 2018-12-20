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

#ifndef RMW_MICROXRCEDDS_H_
#define RMW_MICROXRCEDDS_H_

#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw/get_node_info_and_types.h"
#include "rmw/get_topic_names_and_types.h"
#include "rmw/get_service_names_and_types.h"

const char * rmw_get_implementation_identifier(void);

// How do we pass transport to use?.
rmw_ret_t rmw_init(const rmw_init_options_t * options, rmw_context_t * context);

rmw_node_t * rmw_create_node(
  rmw_context_t * context,
  const char * name,
  const char * namespace,
  size_t domain_id,
  const rmw_node_security_options_t * security_options);

rmw_ret_t rmw_destroy_node(rmw_node_t * node);

const rmw_guard_condition_t * rmw_node_get_graph_guard_condition(const rmw_node_t * node);

rmw_publisher_t * rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_policies);

rmw_ret_t rmw_destroy_publisher(
  rmw_node_t * node,
  rmw_publisher_t * publisher);

rmw_ret_t rmw_publish(
  const rmw_publisher_t * publisher,
  const void * ros_message);

rmw_ret_t rmw_publish_serialized_message(
  const rmw_publisher_t * publisher,
  const rmw_serialized_message_t * serialized_message);

rmw_ret_t rmw_serialize(
  const void * ros_message,
  const rosidl_message_type_support_t * type_support,
  rmw_serialized_message_t * serialized_message);

rmw_ret_t rmw_deserialize(
  const rmw_serialized_message_t * serialized_message,
  const rosidl_message_type_support_t * type_support,
  void * ros_message);

rmw_subscription_t * rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_policies,
  bool ignore_local_publications);

rmw_ret_t rmw_destroy_subscription(
  rmw_node_t * node,
  rmw_subscription_t * subscription);

rmw_ret_t rmw_take(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken);

rmw_ret_t rmw_take_with_info(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken,
  rmw_message_info_t * message_info);

rmw_ret_t rmw_take_serialized_message(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message,
  bool * taken);

rmw_ret_t rmw_take_serialized_message_with_info(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message,
  bool * taken,
  rmw_message_info_t * message_info);

rmw_client_t * rmw_create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_policies);

rmw_ret_t rmw_destroy_client(
  rmw_node_t * node,
  rmw_client_t * client);

rmw_ret_t rmw_send_request(
  const rmw_client_t * client,
  const void * ros_request,
  int64_t * sequence_id);

rmw_ret_t rmw_take_response(
  const rmw_client_t * client,
  rmw_request_id_t * request_header,
  void * ros_response,
  bool * taken);

rmw_service_t * rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_policies);

rmw_ret_t rmw_destroy_service(
  rmw_node_t * node,
  rmw_service_t * service);

rmw_ret_t rmw_take_request(
  const rmw_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_request,
  bool * taken);

rmw_ret_t rmw_send_response(
  const rmw_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_response);

rmw_guard_condition_t * rmw_create_guard_condition(rmw_context_t * context);

rmw_ret_t rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition);

rmw_ret_t rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition);

rmw_wait_set_t * rmw_create_wait_set(size_t max_conditions);

rmw_ret_t rmw_destroy_wait_set(rmw_wait_set_t * wait_set);

rmw_ret_t rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_wait_set_t * wait_set,
  const rmw_time_t * wait_timeout);

rmw_ret_t rmw_get_node_names(
  const rmw_node_t * node,
  rcutils_string_array_t * node_names,
  rcutils_string_array_t * node_namespaces);

rmw_ret_t rmw_count_publishers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count);

rmw_ret_t rmw_count_subscribers(
  const rmw_node_t * node,
  const char * topic_name,
  size_t * count);

rmw_ret_t rmw_get_gid_for_publisher(
  const rmw_publisher_t * publisher,
  rmw_gid_t * gid);

rmw_ret_t rmw_compare_gids_equal(
  const rmw_gid_t * gid1,
  const rmw_gid_t * gid2,
  bool * result);

rmw_ret_t rmw_service_server_is_available(
  const rmw_node_t * node,
  const rmw_client_t * client,
  bool * is_available);

rmw_ret_t rmw_set_log_severity(rmw_log_severity_t severity);

rmw_ret_t rmw_get_topic_names_and_types(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  bool no_demangle,
  rmw_names_and_types_t * topic_names_and_types);

rmw_ret_t rmw_get_service_names_and_types(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  rmw_names_and_types_t * service_names_and_types);

rmw_ret_t
rmw_init_options_init(rmw_init_options_t * init_options, rcutils_allocator_t allocator);

rmw_ret_t
rmw_subscription_count_matched_publishers(
  const rmw_subscription_t * subscription,
  size_t * publisher_count);

rmw_ret_t
rmw_get_publisher_names_and_types_by_node(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  bool demangle,
  rmw_names_and_types_t * topic_names_and_types);

rmw_ret_t
rmw_get_subscriber_names_and_types_by_node(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  bool demangle,
  rmw_names_and_types_t * topics_names_and_types);

rmw_ret_t
rmw_shutdown(rmw_context_t * context);

rmw_ret_t
rmw_init_options_copy(const rmw_init_options_t * src, rmw_init_options_t * dst);

RMW_PUBLIC
rmw_ret_t
rmw_get_service_names_and_types_by_node(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * node_name,
  const char * node_namespace,
  rmw_names_and_types_t * service_names_and_types);

rmw_ret_t
rmw_init_options_fini(rmw_init_options_t * init_options);

rmw_ret_t
rmw_publisher_count_matched_subscriptions(
  const rmw_publisher_t * publisher,
  size_t * subscription_count);

#endif  // RMW_MICROXRCEDDS_H_
