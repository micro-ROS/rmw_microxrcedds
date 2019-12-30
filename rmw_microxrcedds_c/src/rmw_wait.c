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

  if ((NULL == subscriptions) || (0 == subscriptions->subscriber_count)) {
//    return RMW_RET_INVALID_ARGUMENT;
    return RMW_RET_OK; // TODO (julian): review rcl_wait without subscriptions.
  }

  CustomSubscription * custom_subscription = (CustomSubscription *)subscriptions->subscribers[0];
  if (NULL == custom_subscription) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  CustomNode * custom_node = custom_subscription->owner_node;
  if (NULL == custom_node) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (!uxr_run_session_until_timeout(&custom_node->session, (int)timeout)) {
    return RMW_RET_TIMEOUT;
  }

  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}
