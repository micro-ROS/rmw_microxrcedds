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

#include <limits.h>
#include <math.h>

#include <rmw/rmw.h>
#include <rmw/error_handling.h>

#include "./utils.h"

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
  (void)services;
  (void)clients;
  (void)events;
  (void)wait_set;

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

  // Run every XRCE session

  uint8_t available_contexts = 0;
  rmw_uxrce_mempool_item_t * item = NULL;

  item = session_memory.allocateditems;
  while (item != NULL) {
    item = item->next;
    available_contexts++;
  }

  uint64_t per_session_timeout = (uint64_t)((float)timeout / (float)available_contexts);
  item = session_memory.allocateditems;
  while (item != NULL) {
    rmw_context_impl_t * custom_context = (rmw_context_impl_t *)item->data;
    uxr_run_session_until_data(&custom_context->session, per_session_timeout);
    item = item->next;
  }

  bool buffered_status = false;

  // Check services
  if (services) {
    for (size_t i = 0; i < services->service_count; ++i) {
      rmw_uxrce_service_t * custom_service = (rmw_uxrce_service_t *)services->services[i];

      if (NULL == rmw_uxrce_find_static_input_buffer_by_owner((void *) custom_service)) {
        services->services[i] = NULL;
      } else {
        buffered_status = true;
      }
    }
  }

  // Check clients
  if (clients) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      rmw_uxrce_client_t * custom_client = (rmw_uxrce_client_t *)clients->clients[i];

      if (NULL == rmw_uxrce_find_static_input_buffer_by_owner((void *) custom_client)) {
        clients->clients[i] = NULL;
      } else {
        buffered_status = true;
      }
    }
  }

  // Check subscriptions
  if (subscriptions) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      rmw_uxrce_subscription_t * custom_subscription =
        (rmw_uxrce_subscription_t *)subscriptions->subscribers[i];

      if (NULL == rmw_uxrce_find_static_input_buffer_by_owner((void *) custom_subscription)) {
        subscriptions->subscribers[i] = NULL;
      } else {
        buffered_status = true;
      }
    }
  }

  // Check guard conditions
  if (guard_conditions) {
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      bool * hasTriggered = (bool *)guard_conditions->guard_conditions[i];
      if ((*hasTriggered) == false) {
        guard_conditions->guard_conditions[i] = NULL;
      } else {
        *hasTriggered = false;
        buffered_status = true;
      }
    }
  }

  return (buffered_status) ? RMW_RET_OK : RMW_RET_TIMEOUT;
}
