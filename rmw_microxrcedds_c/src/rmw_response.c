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
    const rmw_service_t* service,
    rmw_request_id_t* request_header,
    void* ros_response)
{
    EPROS_PRINT_TRACE();

    if (!is_uxrce_rmw_identifier_valid(service->implementation_identifier))
    {
        RMW_SET_ERROR_MSG("Wrong implementation");
        return(RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
    }

    rmw_uxrce_service_t* custom_service = (rmw_uxrce_service_t*)service->data;
    rmw_uxrce_node_t*    custom_node    = custom_service->owner_node;

    // Conversion from rmw_request_id_t to SampleIdentity
    SampleIdentity sample_id;
    sample_id.sequence_number.high            = (int32_t)(request_header->sequence_number >> 32);
    sample_id.sequence_number.low             = (uint32_t)request_header->sequence_number & 0xFFFFFFFF;
    sample_id.writer_guid.entityId.entityKind = (uint8_t)request_header->writer_guid[0];
    memcpy(
        sample_id.writer_guid.entityId.entityKey, &request_header->writer_guid[1],
        sizeof(sample_id.writer_guid.entityId.entityKey));
    memcpy(
        sample_id.writer_guid.guidPrefix.data, &request_header->writer_guid[4],
        sizeof(sample_id.writer_guid.guidPrefix.data));

    const rosidl_message_type_support_t* res_members =
        custom_service->type_support_callbacks->response_members_();
    const message_type_support_callbacks_t* functions =
        (const message_type_support_callbacks_t*)res_members->data;

    ucdrBuffer mb;
    uint32_t response_length = functions->get_serialized_size(ros_response) + 24; // Adding sample indentity size 
    uint16_t rc = uxr_prepare_output_stream(
                &custom_node->context->session,
                custom_service->stream_id, custom_service->service_id, &mb,
                response_length);
    
    uxr_serialize_SampleIdentity(&mb, &sample_id);
    functions->cdr_serialize(ros_response, &mb);

    if (UXR_INVALID_REQUEST_ID == rc)
    {
        RMW_SET_ERROR_MSG("Micro XRCE-DDS service response error.");
        return(RMW_RET_ERROR);
    }

    return(RMW_RET_OK);
}

rmw_ret_t
rmw_take_response(
    const rmw_client_t* client,
    rmw_service_info_t* request_header,
    void* ros_response,
    bool* taken)
{
    EPROS_PRINT_TRACE();

    if (taken != NULL)
    {
        *taken = false;
    }

    if (!is_uxrce_rmw_identifier_valid(client->implementation_identifier))
    {
        RMW_SET_ERROR_MSG("Wrong implementation");
        return(RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
    }

    rmw_uxrce_client_t* custom_client = (rmw_uxrce_client_t*)client->data;

    if (!custom_client->micro_buffer_in_use)
    {
        return(RMW_RET_ERROR);
    }

    request_header->request_id.sequence_number =
        custom_client->reply_id[custom_client->history_read_index];

    const rosidl_message_type_support_t* res_members =
        custom_client->type_support_callbacks->response_members_();
    const message_type_support_callbacks_t* functions =
        (const message_type_support_callbacks_t*)res_members->data;


    ucdrBuffer temp_buffer;
    ucdr_init_buffer(
        &temp_buffer, custom_client->micro_buffer[custom_client->history_read_index],
        custom_client->micro_buffer_lenght[custom_client->history_read_index]);

    bool deserialize_rv = functions->cdr_deserialize(
        &temp_buffer,
        ros_response);

    custom_client->history_read_index = (custom_client->history_read_index + 1) %
                                        RMW_UXRCE_MAX_HISTORY;
    if (custom_client->history_write_index == custom_client->history_read_index)
    {
        custom_client->micro_buffer_in_use = false;
    }

    if (taken != NULL)
    {
        *taken = deserialize_rv;
    }

    if (!deserialize_rv)
    {
        RMW_SET_ERROR_MSG("Typesupport desserialize error.");
        return(RMW_RET_ERROR);
    }

    EPROS_PRINT_TRACE()
    return(RMW_RET_OK);
}
