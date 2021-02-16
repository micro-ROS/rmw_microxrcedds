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

#ifdef HAVE_C_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#endif
#ifdef HAVE_CPP_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_cpp/identifier.h>
#endif

#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include "./utils.h"

rmw_client_t*
rmw_create_client(
    const rmw_node_t* node,
    const rosidl_service_type_support_t* type_support,
    const char* service_name,
    const rmw_qos_profile_t* qos_policies)
{
    EPROS_PRINT_TRACE()
    rmw_client_t * rmw_client = NULL;
    if (!node)
    {
        RMW_SET_ERROR_MSG("node handle is null");
    }
    else if (!type_support)
    {
        RMW_SET_ERROR_MSG("type support is null");
    }
    else if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier))
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
    }
    else if (!service_name || strlen(service_name) == 0)
    {
        RMW_SET_ERROR_MSG("service name is null or empty string");
    }
    else if (!qos_policies)
    {
        RMW_SET_ERROR_MSG("qos_profile is null");
    }
    else
    {
        rmw_client = (rmw_client_t*)rmw_allocate(
            sizeof(rmw_client_t));
        rmw_client->data = NULL;
        rmw_client->implementation_identifier = rmw_get_implementation_identifier();

        rmw_client->service_name = (const char*)(rmw_allocate(
                                                     sizeof(char) * (strlen(
                                                                         service_name) + 1)));
        if (!rmw_client->service_name)
        {
            RMW_SET_ERROR_MSG("failed to allocate memory");
            goto fail;
        }
        memcpy((void*)rmw_client->service_name, service_name, strlen(service_name) + 1);

        rmw_uxrce_node_t*         custom_node = (rmw_uxrce_node_t*)node->data;
        rmw_uxrce_mempool_item_t* memory_node = get_memory(&client_memory);
        if (!memory_node)
        {
            RMW_SET_ERROR_MSG("Not available memory node");
            goto fail;
        }

        rmw_uxrce_client_t* custom_client = (rmw_uxrce_client_t*)memory_node->data;
        custom_client->rmw_handle = rmw_client;

        custom_client->owner_node = custom_node;
        custom_client->client_gid.implementation_identifier =
            rmw_get_implementation_identifier();
        custom_client->history_write_index = 0;
        custom_client->history_read_index  = 0;

        const rosidl_service_type_support_t* type_support_xrce = NULL;
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE
        type_support_xrce = get_service_typesupport_handle(
            type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);
#endif
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE
        if (NULL == type_support_xrce)
        {
            type_support_xrce = get_service_typesupport_handle(
                type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE);
        }
#endif
        if (NULL == type_support_xrce)
        {
            RMW_SET_ERROR_MSG("Undefined type support");
            goto fail;
        }

        custom_client->type_support_callbacks =
            (const service_type_support_callbacks_t*)type_support_xrce->data;

        if (custom_client->type_support_callbacks == NULL)
        {
            RMW_SET_ERROR_MSG("type support data is NULL");
            goto fail;
        }
        else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE)
        {
            RMW_SET_ERROR_MSG("Not enough memory for impl ids");
            goto fail;
        }


        custom_client->client_id =
            uxr_object_id(custom_node->context->id_requester++, UXR_REQUESTER_ID);

        memset(custom_client->client_gid.data, 0, RMW_GID_STORAGE_SIZE);
        memcpy(
            custom_client->client_gid.data, &custom_client->client_id,
            sizeof(uxrObjectId));

        uint16_t client_req = UXR_INVALID_REQUEST_ID;

#ifdef RMW_UXRCE_TRANSPORT_USE_XML
        char service_name_id[20];
        generate_name(&custom_client->client_id, service_name_id, sizeof(service_name_id));
        if (!build_service_xml(
                service_name_id, service_name, true,
                custom_client->type_support_callbacks, qos_policies, rmw_uxrce_xml_buffer, sizeof(rmw_uxrce_xml_buffer)))
        {
            RMW_SET_ERROR_MSG("failed to generate xml request for client creation");
            goto fail;
        }
        client_req = uxr_buffer_create_requester_xml(
            &custom_node->context->session,
            custom_node->context->reliable_output, custom_client->client_id,
            custom_node->participant_id, rmw_uxrce_xml_buffer, UXR_REPLACE);
#elif defined(RMW_UXRCE_TRANSPORT_USE_REFS)
        // TODO(pablogs9): Is possible to instantiate a replier by ref?
        // client_req = uxr_buffer_create_replier_ref(&custom_node->context->session,
        //     custom_node->context->reliable_output, custom_service->subscriber_id,
        //     custom_node->participant_id, "", UXR_REPLACE);
#endif

        rmw_client->data = custom_client;

        if (!run_xrce_session(custom_node->context, client_req))
        {
            put_memory(&client_memory, &custom_client->mem);
            goto fail;
        }

        uxrDeliveryControl delivery_control;
        delivery_control.max_samples          = UXR_MAX_SAMPLES_UNLIMITED;
        delivery_control.min_pace_period      = 0;
        delivery_control.max_elapsed_time     = UXR_MAX_ELAPSED_TIME_UNLIMITED;
        delivery_control.max_bytes_per_second = UXR_MAX_BYTES_PER_SECOND_UNLIMITED;

        custom_client->stream_id =
            (qos_policies->reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
            custom_node->context->best_effort_input :
            custom_node->context->reliable_input;

        custom_client->request_id = uxr_buffer_request_data(
            &custom_node->context->session,
            custom_node->context->reliable_output, custom_client->client_id,
            custom_client->stream_id, &delivery_control);
    }
    return(rmw_client);

fail:
    rmw_uxrce_fini_client_memory(rmw_client);
    rmw_client = NULL;
    return(rmw_client);
}

rmw_ret_t
rmw_destroy_client(
    rmw_node_t* node,
    rmw_client_t* client)
{
    EPROS_PRINT_TRACE()
    rmw_ret_t result_ret = RMW_RET_OK;
    if (!node)
    {
        RMW_SET_ERROR_MSG("node handle is null");
        result_ret = RMW_RET_ERROR;
    }
    else if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier))
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
        result_ret = RMW_RET_ERROR;
    }
    else if (!node->data)
    {
        RMW_SET_ERROR_MSG("node imp is null");
        result_ret = RMW_RET_ERROR;
    }
    else if (!client)
    {
        RMW_SET_ERROR_MSG("client handle is null");
        result_ret = RMW_RET_ERROR;
    }
    else if (!is_uxrce_rmw_identifier_valid(client->implementation_identifier))
    {
        RMW_SET_ERROR_MSG("client handle not from this implementation");
        result_ret = RMW_RET_ERROR;
    }
    else if (!client->data)
    {
        RMW_SET_ERROR_MSG("client imp is null");
        result_ret = RMW_RET_ERROR;
    }
    else
    {
        rmw_uxrce_node_t*   custom_node   = (rmw_uxrce_node_t*)node->data;
        rmw_uxrce_client_t* custom_client = (rmw_uxrce_client_t*)client->data;
        uint16_t            delete_client =
            uxr_buffer_delete_entity(
                &custom_node->context->session,
                *custom_node->context->creation_destroy_stream,
                custom_client->client_id);

        if (!run_xrce_session(custom_node->context, delete_client))
        {
            result_ret = RMW_RET_ERROR;
        }
        else
        {
            rmw_uxrce_fini_client_memory(client);
            result_ret = RMW_RET_OK;
        }
    }

    return(result_ret);
}
