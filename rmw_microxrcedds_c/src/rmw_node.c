// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "rmw_node.h"  // NOLINT
#include "types.h"
#include "utils.h"
#include "identifiers.h"

#include <rmw_microxrcedds_c/config.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

#include "./types.h"
#include "./utils.h"

rmw_node_t* create_node(
    const char* name, const char* namespace_, size_t domain_id,
    const rmw_context_t* context)
{
    rmw_node_t* node_handle = NULL;

    if (!context)
    {
        RMW_SET_ERROR_MSG("context is null");
        return(NULL);
    }

    rmw_uxrce_mempool_item_t* memory_node = get_memory(&node_memory);
    if (!memory_node)
    {
        RMW_SET_ERROR_MSG("Not available memory node");
        goto fail;
    }

    rmw_uxrce_node_t* node_info = (rmw_uxrce_node_t*)memory_node->data;

    node_info->context = context->impl;

    node_handle = rmw_node_allocate();
    if (!node_handle)
    {
        RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
        return(NULL);
    }

    node_info->rmw_handle = node_handle;

    node_handle->implementation_identifier = rmw_get_implementation_identifier();
    node_handle->data = node_info;
    node_handle->name = (const char*)(rmw_allocate(sizeof(char) * (strlen(name) + 1)));
    if (!node_handle->name)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        rmw_uxrce_fini_node_memory(node_handle);
        return(NULL);
    }
    memcpy((char*)node_handle->name, name, strlen(name) + 1);

    node_handle->namespace_ = rmw_allocate(sizeof(char) * (strlen(namespace_) + 1));
    if (!node_handle->namespace_)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        rmw_uxrce_fini_node_memory(node_handle);
        return(NULL);
    }
    memcpy((char*)node_handle->namespace_, namespace_, strlen(namespace_) + 1);

    node_info->participant_id =
        uxr_object_id(node_info->context->id_participant++, UXR_PARTICIPANT_ID);
    uint16_t participant_req = UXR_INVALID_REQUEST_ID;

#ifdef RMW_UXRCE_TRANSPORT_USE_XML
    if (!build_participant_xml(domain_id, name, rmw_uxrce_xml_buffer, sizeof(rmw_uxrce_xml_buffer)))
    {
        RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
        return(NULL);
    }
    participant_req =
        uxr_buffer_create_participant_xml(
            &node_info->context->session,
            node_info->context->reliable_output,
            node_info->participant_id, (uint16_t)domain_id, rmw_uxrce_xml_buffer, UXR_REPLACE);
#elif defined(RMW_UXRCE_TRANSPORT_USE_REFS)
    if (!build_participant_profile(rmw_uxrce_profile_name, sizeof(rmw_uxrce_profile_name)))
    {
        RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
        return(NULL);
    }
    participant_req =
        uxr_buffer_create_participant_ref(
            &node_info->context->session,
            node_info->context->reliable_output,
            node_info->participant_id, (uint16_t)domain_id, rmw_uxrce_profile_name, UXR_REPLACE);
#endif

    if (!run_xrce_session(node_info->context, participant_req))
    {
        rmw_uxrce_fini_node_memory(node_handle);
        return(NULL);
    }

    return(node_handle);

fail:
    if (node_handle != NULL)
    {
        rmw_uxrce_fini_node_memory(node_handle);
    }
    node_handle = NULL;
    return(node_handle);
}

rmw_node_t*
rmw_create_node(
    rmw_context_t* context,
    const char* name,
    const char* namespace_,
    size_t domain_id,
    bool localhost_only)
{
    (void)context;
    (void)localhost_only;
    EPROS_PRINT_TRACE()
    rmw_node_t * rmw_node = NULL;
    if (!name || strlen(name) == 0)
    {
        RMW_SET_ERROR_MSG("name is null");
    }
    else if (!namespace_ || strlen(namespace_) == 0)
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
    }
    else
    {
        rmw_node = create_node(name, namespace_, domain_id, context);
    }
    return(rmw_node);
}

rmw_ret_t rmw_destroy_node(rmw_node_t* node)
{
    EPROS_PRINT_TRACE()
    rmw_ret_t ret = RMW_RET_OK;
    if (!node)
    {
        RMW_SET_ERROR_MSG("node handle is null");
        return(RMW_RET_ERROR);
    }

    if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier))
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
        return(RMW_RET_ERROR);
    }

    if (!node->data)
    {
        RMW_SET_ERROR_MSG("node impl is null");
        return(RMW_RET_ERROR);
    }

    rmw_uxrce_node_t* custom_node = (rmw_uxrce_node_t*)node->data;
    // TODO(Pablo) make sure that other entities are removed from the pools

    rmw_uxrce_mempool_item_t* item = NULL;

    item = publisher_memory.allocateditems;
    while (item != NULL)
    {
        rmw_uxrce_publisher_t* custom_publisher = (rmw_uxrce_publisher_t*)item->data;
        item = item->next;
        if (custom_publisher->owner_node == custom_node)
        {
            ret = rmw_destroy_publisher(node, custom_publisher->rmw_handle);
        }
    }

    item = subscription_memory.allocateditems;
    while (item != NULL)
    {
        rmw_uxrce_subscription_t* custom_subscription = (rmw_uxrce_subscription_t*)item->data;
        item = item->next;
        if (custom_subscription->owner_node == custom_node)
        {
            ret = rmw_destroy_subscription(node, custom_subscription->rmw_handle);
        }
    }

    item = service_memory.allocateditems;
    while (item != NULL)
    {
        rmw_uxrce_service_t* custom_service = (rmw_uxrce_service_t*)item->data;
        item = item->next;
        if (custom_service->owner_node == custom_node)
        {
            ret = rmw_destroy_service(node, custom_service->rmw_handle);
        }
    }

    item = client_memory.allocateditems;
    while (item != NULL)
    {
        rmw_uxrce_client_t* custom_client = (rmw_uxrce_client_t*)item->data;
        item = item->next;
        if (custom_client->owner_node == custom_node)
        {
            ret = rmw_destroy_client(node, custom_client->rmw_handle);
        }
    }

    uint16_t delete_participant = uxr_buffer_delete_entity(
        &custom_node->context->session,
        *custom_node->context->creation_destroy_stream,
        custom_node->participant_id);

    if (!run_xrce_session(custom_node->context, delete_participant))
    {
        ret = RMW_RET_ERROR;
    }

    rmw_uxrce_fini_node_memory(node);

    return(ret);
}

rmw_ret_t
rmw_node_assert_liveliness(const rmw_node_t* node)
{
    (void)node;
    RMW_SET_ERROR_MSG("function not implemented");
    return(RMW_RET_UNSUPPORTED);
}

const rmw_guard_condition_t*
rmw_node_get_graph_guard_condition(const rmw_node_t* node)
{
    rmw_uxrce_node_t*      custom_node           = (rmw_uxrce_node_t*)node->data;
    rmw_context_impl_t*    context               = custom_node->context;
    rmw_guard_condition_t* graph_guard_condition =
        &context->graph_guard_condition;

#ifdef RMW_UXRCE_GRAPH
    if (NULL == graph_guard_condition->data)
    {
        graph_guard_condition->data = (void*)(&context->graph_info.has_changed);
    }
#endif  // RMW_UXRCE_GRAPH

    return(graph_guard_condition);
}
