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

#include "./rmw_microxrcedds.h"  // NOLINT

#include <limits.h>
#include <time.h>

#include <uxr/client/client.h>
#include <rosidl_typesupport_microxrcedds_shared/identifier.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"

#include "./identifier.h"

#include "./rmw_node.h"
#include "./rmw_publisher.h"
#include "./rmw_subscriber.h"
#include "./types.h"
#include "./utils.h"


const char * rmw_get_implementation_identifier()
{
  EPROS_PRINT_TRACE()
  return eprosima_microxrcedds_identifier;
}

rmw_ret_t rmw_init()
{
  EPROS_PRINT_TRACE()

  // Intialize random number generator
  time_t t;
  srand((unsigned)time(&t));

  init_rmw_node();

  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_node_t * rmw_create_node(
  const char * name, const char * namespace, size_t domain_id,
  const rmw_node_security_options_t * security_options)
{
  EPROS_PRINT_TRACE()
  rmw_node_t * rmw_node = NULL;
  if (!name || strlen(name) == 0) {
    RMW_SET_ERROR_MSG("name is null");
  } else if (!namespace || strlen(namespace) == 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!security_options) {
    RMW_SET_ERROR_MSG("security_options is null");
  } else {
    rmw_node = create_node(name, namespace, domain_id);
  }
  return rmw_node;
}

const rmw_guard_condition_t * rmw_node_get_graph_guard_condition(const rmw_node_t * node)
{
  (void)node;
  EPROS_PRINT_TRACE()
  rmw_guard_condition_t *
  ret = (rmw_guard_condition_t *)rmw_allocate(sizeof(rmw_guard_condition_t));
  ret->data = NULL;
  ret->implementation_identifier = eprosima_microxrcedds_identifier;
  return ret;
}

rmw_publisher_t * rmw_create_publisher(
  const rmw_node_t * node, const rosidl_message_type_support_t * type_support,
  const char * topic_name, const rmw_qos_profile_t * qos_policies)
{
  EPROS_PRINT_TRACE()
  rmw_publisher_t * rmw_publisher = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("publisher topic is null or empty string");
    return NULL;
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return NULL;
  } else {
    rmw_publisher = create_publisher(node, type_support, topic_name, qos_policies);
  }
  return rmw_publisher;
}

rmw_ret_t rmw_publish_serialized_message(
  const rmw_publisher_t * publisher,
  const rmw_serialized_message_t * serialized_message)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_serialize(
  const void * ros_message, const rosidl_message_type_support_t * type_support,
  rmw_serialized_message_t * serialized_message)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_deserialize(
  const rmw_serialized_message_t * serialized_message,
  const rosidl_message_type_support_t * type_support, void * ros_message)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_subscription_t * rmw_create_subscription(
  const rmw_node_t * node, const rosidl_message_type_support_t * type_support,
  const char * topic_name, const rmw_qos_profile_t * qos_policies,
  bool ignore_local_publications)
{
  EPROS_PRINT_TRACE()
  rmw_subscription_t * rmw_subscription = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (strcmp(type_support->typesupport_identifier,  // NOLINT
    ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE) != 0)
  {
    RMW_SET_ERROR_MSG("TypeSupport handle not from this implementation");
  } else if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("subscription topic is null or empty string");
    return NULL;
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return NULL;
  } else {
    rmw_subscription = create_subscriber(node, type_support, topic_name, qos_policies,
        ignore_local_publications);
  }

  return rmw_subscription;
}

rmw_ret_t rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  return rmw_take_with_info(subscription, ros_message, taken, NULL);
}

rmw_ret_t rmw_take_with_info(
  const rmw_subscription_t * subscription, void * ros_message, bool * taken,
  rmw_message_info_t * message_info)
{
  EPROS_PRINT_TRACE()
  // Not used variables
    (void) message_info;

  // Preconfigure taken
  if (taken != NULL) {
    *taken = false;
  }

  // Check id
  if (strcmp(subscription->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_ERROR;
  }

  // Extract subscriber info
  CustomSubscription * custom_subscription = (CustomSubscription *)subscription->data;


  // Extract serialiced message using typesupport
  bool deserialize_rv = custom_subscription->type_support_callbacks->cdr_deserialize(
    &custom_subscription->micro_buffer, ros_message,
    custom_subscription->owner_node->miscellaneous_temp_buffer,
    sizeof(custom_subscription->owner_node->miscellaneous_temp_buffer));
  if (taken != NULL) {
    *taken = deserialize_rv;
  }
  if (!deserialize_rv) {
    RMW_SET_ERROR_MSG("Typesupport desserialize error.");
    return RMW_RET_ERROR;
  }

  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_take_serialized_message(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message, bool * taken)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_take_serialized_message_with_info(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message, bool * taken,
  rmw_message_info_t * message_info)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_client_t * rmw_create_client(
  const rmw_node_t * node, const rosidl_service_type_support_t * type_support,
  const char * service_name, const rmw_qos_profile_t * qos_policies)
{
  EPROS_PRINT_TRACE()
  
  rmw_client_t * rmw_client = (rmw_client_t *)rmw_allocate(
    sizeof(rmw_client_t));
  return rmw_client;
}

rmw_ret_t rmw_destroy_client(rmw_node_t * node, rmw_client_t * client)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_send_request(
  const rmw_client_t * client, const void * ros_request,
  int64_t * sequence_id)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_take_response(
  const rmw_client_t * client, rmw_request_id_t * request_header, void * ros_response,
  bool * taken)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_service_t * rmw_create_service(
  const rmw_node_t * node, const rosidl_service_type_support_t * type_support,
  const char * service_name, const rmw_qos_profile_t * qos_policies)
{
  EPROS_PRINT_TRACE()

  rmw_service_t * rmw_service = (rmw_service_t *)rmw_allocate(
    sizeof(rmw_service_t));
  return rmw_service;
}

rmw_ret_t rmw_destroy_service(rmw_node_t * node, rmw_service_t * service)
{
  (void) node;

  EPROS_PRINT_TRACE()

  rmw_free(service);

  return RMW_RET_OK;
}

rmw_ret_t rmw_take_request(
  const rmw_service_t * service, rmw_request_id_t * request_header, void * ros_request,
  bool * taken)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_send_response(
  const rmw_service_t * service, rmw_request_id_t * request_header,
  void * ros_response)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_guard_condition_t * rmw_create_guard_condition(void)
{
  EPROS_PRINT_TRACE()

  rmw_guard_condition_t * rmw_guard_condition = (rmw_guard_condition_t *)rmw_allocate(
    sizeof(rmw_guard_condition_t));

  return rmw_guard_condition;
}

rmw_ret_t rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  EPROS_PRINT_TRACE()

  rmw_free(guard_condition);

  return RMW_RET_OK;
}

rmw_ret_t rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_wait_set_t * rmw_create_wait_set(size_t max_conditions)
{
  EPROS_PRINT_TRACE()

  rmw_wait_set_t * rmw_wait_set = (rmw_wait_set_t *)rmw_allocate(
    sizeof(rmw_wait_set_t));

  return rmw_wait_set;
}

rmw_ret_t rmw_destroy_wait_set(rmw_wait_set_t * wait_set)
{
  EPROS_PRINT_TRACE()

  rmw_free(wait_set);

  return RMW_RET_OK;
}

rmw_ret_t rmw_wait(
  rmw_subscriptions_t * subscriptions, rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services, rmw_clients_t * clients, rmw_wait_set_t * wait_set,
  const rmw_time_t * wait_timeout)
{
  EPROS_PRINT_TRACE()
  // Wait set is not used
    (void) wait_set;

  // for Subscription requests and response
  uint16_t subscription_request[MAX_SUBSCRIPTIONS_X_NODE];
  uint8_t subscription_status_request[MAX_SUBSCRIPTIONS_X_NODE];

  // Go throw all subscriptions
  CustomNode * custom_node = NULL;
  size_t subscriber_requests_count = 0;
  if ((subscriptions != NULL) && (subscriptions->subscriber_count > 0)) {
    // Extract first session pointer
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      if (subscriptions->subscribers[i] != NULL) {
        CustomSubscription * custom_subscription =
          (CustomSubscription *)subscriptions->subscribers[i];
        custom_node = custom_subscription->owner_node;


        if (custom_subscription->waiting_for_response == false) {
          custom_subscription->waiting_for_response = true;
          custom_subscription->subscription_request = uxr_buffer_request_data(&custom_node->session,
              custom_node->reliable_output, custom_subscription->datareader_id,
              custom_node->reliable_input,
              NULL);
        }


        // Reset the request id
        subscription_request[i] = custom_subscription->subscription_request;
      }
    }
  }

  // Go throw all services
  /*
  else if ((services != NULL) && (services->service_count > 0))
  {
      // Extract first session pointer
      //services->services[0];
  }
  */

  // Go throw all clients
  /*
  else if ((clients != NULL) && (clients->client_count > 0))
  {
      // Extract first session pointer
      //clients->clients[0];
  }
  */

  // Check node pointer
  if (custom_node == NULL) {
    if (subscriptions != NULL) {
      for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
        subscriptions->subscribers[i] = NULL;
      }
    }
    if (services != NULL) {
      for (size_t i = 0; i < services->service_count; ++i) {
        services->services[i] = NULL;
      }
    }
    if (clients != NULL) {
      for (size_t i = 0; i < clients->client_count; ++i) {
        clients->clients[i] = NULL;
      }
    }

    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
  }

  // Check if timeout
  uint64_t timeout;
  if (wait_timeout != NULL) {
    // Convert to int (checking overflow)
    if (wait_timeout->sec >= (UINT64_MAX / 1000)) {
      // Overflow
      timeout = INT_MAX;
      RMW_SET_ERROR_MSG("Wait timeout overflow");
    } else {
      timeout = wait_timeout->sec * 1000;

      uint64_t timeout_ms = wait_timeout->nsec / 1000000;
      if ((UINT64_MAX - timeout) <= timeout_ms) {
        // Overflow
        timeout = INT_MAX;
        RMW_SET_ERROR_MSG("Wait timeout overflow");
      } else {
        timeout += timeout_ms;
        if (timeout > INT_MAX) {
          // Overflow
          timeout = INT_MAX;
          RMW_SET_ERROR_MSG("Wait timeout overflow");
        }
      }
    }
  } else {
    timeout = UXR_TIMEOUT_INF;
  }

  // read until status or timeout
  if (subscriptions->subscriber_count > 0) {
    uxr_run_session_until_one_status(&custom_node->session, timeout, subscription_request,
      subscription_status_request, subscriptions->subscriber_count);
  }


  // Clean non-received
  bool is_timeout = true;
  if (subscriptions != NULL) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      // Check if there are any data
      CustomSubscription * custom_subscription =
        (CustomSubscription *)(subscriptions->subscribers[i]);
      if (custom_subscription->waiting_for_response) {
        subscriptions->subscribers[i] = NULL;
      } else {
        is_timeout = false;
      }
    }
  }
  if (services != NULL) {
    for (size_t i = 0; i < services->service_count; ++i) {
      services->services[i] = NULL;
    }
  }
  if (clients != NULL) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      clients->clients[i] = NULL;
    }
  }

  // Check if timeout
  if (is_timeout) {
    EPROS_PRINT_TRACE()
    return RMW_RET_TIMEOUT;
  }

  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_node_names(const rmw_node_t * node, rcutils_string_array_t * node_names)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_count_publishers(const rmw_node_t * node, const char * topic_name, size_t * count)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_count_subscribers(const rmw_node_t * node, const char * topic_name, size_t * count)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_gid_for_publisher(const rmw_publisher_t * publisher, rmw_gid_t * gid)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_compare_gids_equal(const rmw_gid_t * gid1, const rmw_gid_t * gid2, bool * result)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_service_server_is_available(
  const rmw_node_t * node, const rmw_client_t * client,
  bool * is_available)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_set_log_6severity(rmw_log_severity_t severity)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_topic_names_and_types(
  const rmw_node_t * node, rcutils_allocator_t * allocator, bool no_demangle,
  rmw_names_and_types_t * topic_names_and_types)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_service_names_and_types(
  const rmw_node_t * node, rcutils_allocator_t * allocator,
  rmw_names_and_types_t * service_names_and_types)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}
