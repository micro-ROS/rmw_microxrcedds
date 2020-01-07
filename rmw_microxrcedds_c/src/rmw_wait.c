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
#include <rmw/error_handling.h>

#include <limits.h>

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_events_t * events,
  rmw_wait_set_t * wait_set,
  const rmw_time_t * wait_timeout)
{
  (void) services;
  (void) clients;
  (void) events;
  (void) wait_set;
  EPROS_PRINT_TRACE()

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
    timeout = (uint64_t)UXR_TIMEOUT_INF;
  }

  // Check guard conditions
  if (guard_conditions) {
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      bool * hasTriggered = (bool *)guard_conditions->guard_conditions[i];
      if ((*hasTriggered) == false)
      {
        guard_conditions->guard_conditions[i] = NULL;
      }else
      {
        *hasTriggered = false;
      }
    }
  }

  rmw_ret_t ret = RMW_RET_OK;

  // TODO (Pablo): Check this with no subscription or service/client 
  // TODO (Pablo): Iterate along different nodes avaliable.
  CustomNode * custom_node = NULL;
  if (subscriptions->subscriber_count)
  {
    CustomSubscription * first_subscription = (CustomSubscription *)subscriptions->subscribers[0];
    custom_node = first_subscription->owner_node;
  }else if (services->service_count)
  {
    CustomService * first_service = (CustomService *)services->services[0];
    custom_node = first_service->owner_node;
  }else if (clients->client_count)
  {
    CustomClient * first_client = (CustomClient *)clients->clients[0];
    custom_node = first_client->owner_node;
  }

  if (NULL == custom_node) {
    ret = RMW_RET_INVALID_ARGUMENT;
  }
  
  // Run XRCE session
  if (!uxr_run_session_until_timeout(&custom_node->session, (int)timeout)) {
    ret = RMW_RET_TIMEOUT;
  }

  // Check services
  if (services) {
    for (size_t i = 0; i < services->service_count; ++i) {
      CustomService * custom_service = (CustomService *)services->services[i];
      
      if ((MAX_HISTORY > 1 && custom_service->history_read_index == custom_service->history_write_index) || 
          (MAX_HISTORY == 1 && !custom_service->micro_buffer_in_use))
      {
        services->services[i] = NULL;
      }
    }
  }

  // Check clients
  if (clients) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      CustomClient * custom_client = (CustomClient *)clients->clients[i];
      
      if ((MAX_HISTORY > 1 && custom_client->history_read_index == custom_client->history_write_index) || 
          (MAX_HISTORY == 1 && !custom_client->micro_buffer_in_use)) 
      {
        clients->clients[i] = NULL;
      }
    }
  }

  // Check subscriptions
  if (subscriptions) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      CustomSubscription * custom_subscription = (CustomSubscription *)subscriptions->subscribers[i];
      
      if (!custom_subscription->micro_buffer_in_use)
      {
        subscriptions->subscribers[i] = NULL;
      }
    }
  }

  EPROS_PRINT_TRACE()

  // TODO (Pablo): When it need to return a timeout?
  ret = RMW_RET_OK;
  return ret;
}
