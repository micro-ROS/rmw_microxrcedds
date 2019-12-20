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
rmw_send_response(
  const rmw_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_response)
{
  EPROS_PRINT_TRACE();

  if (strcmp(service->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  CustomService * custom_service = (CustomService *)service->data;
  CustomNode * custom_node = (CustomNode *)custom_service->owner_node;

  // Conversion from rmw_request_id_t to SampleIdentity  
  SampleIdentity sample_id;
  sample_id.sequence_number.high = (int32_t) (request_header->sequence_number >> 32);
  sample_id.sequence_number.low = (uint32_t) request_header->sequence_number & 0xFFFFFFFF;
  sample_id.writer_guid.entityId.entityKind = (uint8_t) request_header->writer_guid[0];
  memcpy(sample_id.writer_guid.entityId.entityKey,&request_header->writer_guid[1],sizeof(sample_id.writer_guid.entityId.entityKey));
  memcpy(sample_id.writer_guid.guidPrefix.data,&request_header->writer_guid[4],sizeof(sample_id.writer_guid.guidPrefix.data));

  const rosidl_message_type_support_t * res_members = custom_service->type_support_callbacks->response_members_();
  const message_type_support_callbacks_t * functions = (const message_type_support_callbacks_t *)res_members->data;

  uint32_t topic_size = functions->get_serialized_size(ros_response);

  ucdrBuffer reply_ub;
  ucdr_init_buffer(&reply_ub, custom_service->replay_buffer, sizeof(custom_service->replay_buffer));

  functions->cdr_serialize(ros_response,&reply_ub);
 
  uxr_buffer_reply(&custom_node->session, custom_node->reliable_output, 
      custom_service->service_id, &sample_id, custom_service->replay_buffer, topic_size);

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_response(
  const rmw_client_t * client,
  rmw_request_id_t * request_header,
  void * ros_response,
  bool * taken)
{
  EPROS_PRINT_TRACE();

  if (taken != NULL) {
    *taken = false;
  }

  if (strcmp(client->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  CustomClient * custom_client = (CustomClient *)client->data;

  if ((MAX_HISTORY > 1 && custom_client->history_read_index == custom_client->history_write_index) || 
      (MAX_HISTORY == 1 && !custom_client->micro_buffer_in_use)) {
    return RMW_RET_ERROR;
  }

  request_header->sequence_number = custom_client->reply_id[custom_client->history_read_index];

  const rosidl_message_type_support_t * res_members = custom_client->type_support_callbacks->request_members_();
  const message_type_support_callbacks_t * functions = (const message_type_support_callbacks_t *)res_members->data;


  ucdrBuffer temp_buffer;
  ucdr_init_buffer(&temp_buffer, custom_client->micro_buffer[custom_client->history_read_index], 
                  custom_client->micro_buffer_lenght[custom_client->history_read_index]);

  bool deserialize_rv = functions->cdr_deserialize(
    &temp_buffer,
    ros_response);

  custom_client->history_read_index = (custom_client->history_read_index + 1) % MAX_HISTORY;

  if (MAX_HISTORY == 1){
    custom_client->micro_buffer_in_use = false;
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
