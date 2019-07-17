// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "utils.h"

#include <rmw/rmw.h>

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_wait_set_t * wait_set,
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



