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
#include <rmw/types.h>

rmw_ret_t
rmw_init_subscription_allocation(
  const rosidl_message_type_support_t * type_support,
  const rosidl_message_bounds_t * message_bounds,
  rmw_subscription_allocation_t * allocation)
{
  (void) type_support;
  (void) message_bounds;
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_fini_subscription_allocation(rmw_subscription_allocation_t * allocation)
{
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}

rmw_subscription_t * rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_policies,
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

rmw_ret_t
rmw_subscription_count_matched_publishers(
  const rmw_subscription_t * subscription,
  size_t * publisher_count)
{
  (void) subscription;
  (void) publisher_count;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_subscription_get_actual_qos(
        const rmw_subscription_t * subscription,
        rmw_qos_profile_t * qos)
{
  (void) subscription;
  (void) qos;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
{
  EPROS_PRINT_TRACE()
  rmw_ret_t result_ret = RMW_RET_OK;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!node->data) {
    RMW_SET_ERROR_MSG("node imp is null");
    result_ret = RMW_RET_ERROR;
  } else if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (strcmp(subscription->implementation_identifier,  // NOLINT
    rmw_get_implementation_identifier()) != 0)
  {
    RMW_SET_ERROR_MSG("subscription handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!subscription->data) {
    RMW_SET_ERROR_MSG("subscription imp is null");
    result_ret = RMW_RET_ERROR;
  } else {
    CustomNode * custom_node = (CustomNode *)node->data;
    CustomSubscription * custom_subscription = (CustomSubscription *)subscription->data;
    uint16_t delete_datareader =
      uxr_buffer_delete_entity(&custom_node->session, custom_node->reliable_output,
        custom_subscription->datareader_id);
    uint16_t delete_subscriber =
      uxr_buffer_delete_entity(&custom_node->session, custom_node->reliable_output,
        custom_subscription->subscriber_id);

    uint16_t requests[] = {delete_datareader, delete_subscriber};
    uint8_t status[sizeof(requests) / 2];
    if (!uxr_run_session_until_all_status(&custom_node->session, 1000, requests, status,
      sizeof(status)))
    {
      RMW_SET_ERROR_MSG("unable to remove publisher from the server");
      result_ret = RMW_RET_ERROR;
    } else {
      rmw_subscription_delete(subscription);
      result_ret = RMW_RET_OK;
    }
  }

  return result_ret;
}
