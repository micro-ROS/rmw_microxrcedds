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

#include "rmw_microxrcedds_c/rmw_microxrcedds.h"  // NOLINT

#include <limits.h>

#include <uxr/client/client.h>
#include <rosidl_typesupport_microxrcedds_shared/identifier.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"

#include "./identifiers.h"

#include "./rmw_node.h"
#include "./rmw_publisher.h"
#include "./rmw_subscriber.h"
#include "./types.h"
#include "./utils.h"

rmw_ret_t rmw_publish_serialized_message(
  const rmw_publisher_t * publisher,
  const rmw_serialized_message_t * serialized_message)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
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
  custom_subscription->micro_buffer_in_use = false;
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

rmw_ret_t rmw_get_topic_names_and_types(
  const rmw_node_t * node, rcutils_allocator_t * allocator, bool no_demangle,
  rmw_names_and_types_t * topic_names_and_types)
{
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}


