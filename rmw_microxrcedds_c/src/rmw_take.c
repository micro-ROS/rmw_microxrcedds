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

rmw_ret_t
rmw_take(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken,
  rmw_subscription_allocation_t * allocation)
{
  return rmw_take_with_info(subscription, ros_message, taken, NULL, allocation);
}

rmw_ret_t
rmw_take_with_info(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken,
  rmw_message_info_t * message_info,
  rmw_subscription_allocation_t * allocation)
{
  (void) message_info;
  (void) allocation;

  EPROS_PRINT_TRACE()

  if (taken != NULL) {
    *taken = false;
  }

  if (!is_uxrce_rmw_identifier_valid(subscription->implementation_identifier)) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_ERROR;
  }

  rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)subscription->data;

  if (!custom_subscription->micro_buffer_in_use) {
    return RMW_RET_ERROR;
  }

  ucdrBuffer temp_buffer;
  ucdr_init_buffer(
    &temp_buffer, custom_subscription->micro_buffer[custom_subscription->history_read_index],
    custom_subscription->micro_buffer_lenght[custom_subscription->history_read_index]);

  bool deserialize_rv = custom_subscription->type_support_callbacks->cdr_deserialize(
    &temp_buffer,
    ros_message);

  custom_subscription->history_read_index = (custom_subscription->history_read_index + 1) %
    RMW_UXRCE_MAX_HISTORY;
  if (custom_subscription->history_write_index == custom_subscription->history_read_index) {
    custom_subscription->micro_buffer_in_use = false;
  }

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

rmw_ret_t
rmw_take_sequence(
  const rmw_subscription_t * subscription,
  size_t count,
  rmw_message_sequence_t * message_sequence,
  rmw_message_info_sequence_t * message_info_sequence,
  size_t * taken,
  rmw_subscription_allocation_t * allocation)
{
  bool taken_flag;
  rmw_ret_t ret = RMW_RET_OK;

  *taken = 0;

  if (!is_uxrce_rmw_identifier_valid(subscription->implementation_identifier)) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_ERROR;
  }

  rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)subscription->data;

  if (!custom_subscription->micro_buffer_in_use) {
    return RMW_RET_ERROR;
  }

  for (size_t i = 0; i < count; i++) {
    taken_flag = false;

    ret = rmw_take_with_info(
      subscription,
      message_sequence->data[*taken],
      &taken_flag,
      &message_info_sequence->data[*taken],
      allocation
    );

    if (ret != RMW_RET_OK || !taken_flag) {
      break;
    }

    (*taken)++;
  }

  message_sequence->size = *taken;
  message_info_sequence->size = *taken;

  return ret;
}

rmw_ret_t
rmw_take_serialized_message(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message,
  bool * taken,
  rmw_subscription_allocation_t * allocation)
{
  (void) subscription;
  (void) serialized_message;
  (void) taken;
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_take_serialized_message_with_info(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message,
  bool * taken,
  rmw_message_info_t * message_info,
  rmw_subscription_allocation_t * allocation)
{
  (void) subscription;
  (void) serialized_message;
  (void) taken;
  (void) message_info;
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_take_loaned_message(
  const rmw_subscription_t * subscription,
  void ** loaned_message,
  bool * taken,
  rmw_subscription_allocation_t * allocation)
{
  (void) subscription;
  (void) loaned_message;
  (void) taken;
  (void) allocation;

  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_take_loaned_message_with_info(
  const rmw_subscription_t * subscription,
  void ** loaned_message,
  bool * taken,
  rmw_message_info_t * message_info,
  rmw_subscription_allocation_t * allocation)
{
  (void) subscription;
  (void) loaned_message;
  (void) taken;
  (void) message_info;
  (void) allocation;

  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_return_loaned_message_from_subscription(
  const rmw_subscription_t * subscription,
  void * loaned_message)
{
  (void) subscription;
  (void) loaned_message;

  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_take_event(
  const rmw_events_t * event_handle,
  void * event_info,
  bool * taken)
{
  (void) event_handle;
  (void) event_info;
  (void) taken;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}
