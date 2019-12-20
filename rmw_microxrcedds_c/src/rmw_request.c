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
rmw_send_request(
  const rmw_client_t * client,
  const void * ros_request,
  int64_t * sequence_id)
{
  EPROS_PRINT_TRACE();

  if (strcmp(client->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  CustomClient * custom_client = (CustomClient *)client->data;
  CustomNode * custom_node = (CustomNode *)custom_client->owner_node;
  
  const rosidl_message_type_support_t * req_members = custom_client->type_support_callbacks->response_members_();
  const message_type_support_callbacks_t * functions = (const message_type_support_callbacks_t *)req_members->data;

  uint32_t topic_size = functions->get_serialized_size(ros_request);

  ucdrBuffer request_ub;
  ucdr_init_buffer(&request_ub, custom_client->request_buffer, sizeof(custom_client->request_buffer));

  functions->cdr_serialize(ros_request,&request_ub);
 
  *sequence_id = uxr_buffer_request(&custom_node->session, custom_node->reliable_output, 
      custom_client->client_id, custom_client->request_buffer, topic_size);

  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_request(
  const rmw_service_t * service,
  rmw_request_id_t * request_header,
  void * ros_request,
  bool * taken)
{
  EPROS_PRINT_TRACE();

  if (taken != NULL) {
    *taken = false;
  }

  if (strcmp(service->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("Wrong implementation");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  CustomService * custom_service = (CustomService *)service->data;

  if ((MAX_HISTORY > 1 && custom_service->history_read_index == custom_service->history_write_index) || 
      (MAX_HISTORY == 1 && !custom_service->micro_buffer_in_use)) {
      return RMW_RET_ERROR;
  }

  // Conversion from SampleIdentity to rmw_request_id_t 
  request_header->sequence_number = (((int64_t) custom_service->sample_id[custom_service->history_read_index].sequence_number.high) << 32) | 
                                    custom_service->sample_id[custom_service->history_read_index].sequence_number.low;
  request_header->writer_guid[0] = (int8_t) custom_service->sample_id[custom_service->history_read_index].writer_guid.entityId.entityKind;
  memcpy(&request_header->writer_guid[1],custom_service->sample_id[custom_service->history_read_index].writer_guid.entityId.entityKey,3);
  memcpy(&request_header->writer_guid[4],custom_service->sample_id[custom_service->history_read_index].writer_guid.guidPrefix.data,12);

  const rosidl_message_type_support_t * req_members = custom_service->type_support_callbacks->request_members_();
  const message_type_support_callbacks_t * functions = (const message_type_support_callbacks_t *)req_members->data;

  ucdrBuffer temp_buffer;
  ucdr_init_buffer(&temp_buffer, custom_service->micro_buffer[custom_service->history_read_index], 
                    custom_service->micro_buffer_lenght[custom_service->history_read_index]);


  bool deserialize_rv = functions->cdr_deserialize(&temp_buffer, ros_request);

  custom_service->history_read_index = (custom_service->history_read_index + 1) % MAX_HISTORY;

  if (MAX_HISTORY == 1){
    custom_service->micro_buffer_in_use = false;
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

