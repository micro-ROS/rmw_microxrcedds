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

#include "types.h"
#include "utils.h"

#include <rmw/error_handling.h>
#include <rmw/rmw.h>

rmw_ret_t
rmw_publish(
  const rmw_publisher_t * publisher,
  const void * ros_message,
  rmw_publisher_allocation_t * allocation)
{
  (void) allocation;
  EPROS_PRINT_TRACE()
  rmw_ret_t ret = RMW_RET_OK;
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher pointer is null");
    ret = RMW_RET_ERROR;
  } else if (!ros_message) {
    RMW_SET_ERROR_MSG("ros_message pointer is null");
    ret = RMW_RET_ERROR;
  } else if (strcmp(publisher->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("publisher handle not from this implementation");
    ret = RMW_RET_ERROR;
  } else if (!publisher->data) {
    RMW_SET_ERROR_MSG("publisher imp is null");
    ret = RMW_RET_ERROR;
  } else {
    rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;
    const message_type_support_callbacks_t * functions = custom_publisher->type_support_callbacks;
    uint32_t topic_length = functions->get_serialized_size(ros_message);

    uxrStreamId used_stream_id = 
      (custom_publisher->qos.reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
      custom_node->context->best_effort_output :
      custom_node->context->reliable_output;

    ucdrBuffer mb;
    bool written = false;
    if (uxr_prepare_output_stream(&custom_publisher->owner_node->context->session,
      used_stream_id, custom_publisher->datawriter_id, &mb,
      topic_length))
    {
      ucdrBuffer mb_topic;
      ucdr_init_buffer(&mb_topic, mb.iterator, topic_length);
      written = functions->cdr_serialize(ros_message, &mb_topic);

      if (custom_publisher->qos.reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT){
        uxr_flash_output_streams(&custom_publisher->owner_node->context->session);
      }else{
        written &= uxr_run_session_until_confirm_delivery(&custom_publisher->owner_node->context->session, 1000);
      }
    }
    if (!written) {
      RMW_SET_ERROR_MSG("error publishing message");
      ret = RMW_RET_ERROR;
    }
  }
  return ret;
}

rmw_ret_t
rmw_publish_serialized_message(
  const rmw_publisher_t * publisher,
  const rmw_serialized_message_t * serialized_message,
  rmw_publisher_allocation_t * allocation)
{
  (void) publisher;
  (void) serialized_message;
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}
