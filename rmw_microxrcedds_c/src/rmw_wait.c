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
#include <math.h>

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

  // Look for the XRCE session
  uxrSession * session = NULL;

  for (size_t i = 0; i < services->service_count && session == NULL; i++){
    CustomService * custom_service = (CustomService *)services->services[i];
    session = &custom_service->owner_node->context->session;
  }

  for (size_t i = 0; i < clients->client_count && session == NULL; i++){
    CustomClient * custom_client = (CustomClient *)clients->clients[i];
    session = &custom_client->owner_node->context->session;

  }

  for (size_t i = 0; i < subscriptions->subscriber_count && session == NULL; ++i) {
    CustomSubscription * custom_subscription = (CustomSubscription *)subscriptions->subscribers[i];
    session = &custom_subscription->owner_node->context->session;
  }

  uxr_run_session_until_timeout(session, timeout);

  // Check services
  if (services) {
    for (size_t i = 0; i < services->service_count; ++i) {
      CustomService * custom_service = (CustomService *)services->services[i];
      
      if (!custom_service->micro_buffer_in_use){
        services->services[i] = NULL;
      }
    }
  }

  // Check clients
  if (clients) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      CustomClient * custom_client = (CustomClient *)clients->clients[i];
      
      if (!custom_client->micro_buffer_in_use){
        clients->clients[i] = NULL;
      }
    }
  }

  // Check subscriptions
  if (subscriptions) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      CustomSubscription * custom_subscription = (CustomSubscription *)subscriptions->subscribers[i];
      
      if (!custom_subscription->micro_buffer_in_use){
        subscriptions->subscribers[i] = NULL;
      }
    }
  }

  EPROS_PRINT_TRACE()

  // TODO (Pablo): When it need to return a timeout?
  ret = RMW_RET_OK;
  return ret;
}
