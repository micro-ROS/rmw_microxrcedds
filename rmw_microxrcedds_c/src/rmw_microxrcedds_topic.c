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

#include "./rmw_microxrcedds_topic.h"  // NOLINT

#include <string.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include "./utils.h"


rmw_uxrce_topic_t*
create_topic(
    struct rmw_uxrce_node_t* custom_node,
    const char* topic_name,
    const message_type_support_callbacks_t* message_type_support_callbacks,
    const rmw_qos_profile_t* qos_policies)
{
    rmw_uxrce_topic_t*        custom_topic = NULL;
    rmw_uxrce_mempool_item_t* memory_node  = get_memory(&topics_memory);

    if (!memory_node)
    {
        RMW_SET_ERROR_MSG("Not available memory node");
        goto fail;
    }

    custom_topic = (rmw_uxrce_topic_t*)memory_node->data;

    // Init
    custom_topic->sync_with_agent = false;
    custom_topic->owner_node      = custom_node;

    // Asociate to typesupport
    custom_topic->message_type_support_callbacks = message_type_support_callbacks;

    // Generate topic id
    custom_topic->topic_id = uxr_object_id(custom_node->context->id_topic++, UXR_TOPIC_ID);

    // Generate request
    uint16_t topic_req = 0;
#ifdef RMW_UXRCE_TRANSPORT_USE_XML
    if (!build_topic_xml(
            topic_name, message_type_support_callbacks,
            qos_policies, rmw_uxrce_xml_buffer, sizeof(rmw_uxrce_xml_buffer)))
    {
        RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
        rmw_uxrce_fini_topic_memory(custom_topic);
        custom_topic = NULL;
        goto fail;
    }

    topic_req = uxr_buffer_create_topic_xml(
        &custom_node->context->session,
        custom_node->context->reliable_output, custom_topic->topic_id,
        custom_node->participant_id, rmw_uxrce_xml_buffer, UXR_REPLACE);
#elif defined(RMW_UXRCE_TRANSPORT_USE_REFS)
    (void)qos_policies;
    if (!build_topic_profile(topic_name, rmw_uxrce_profile_name, sizeof(rmw_uxrce_profile_name)))
    {
        RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
        rmw_uxrce_fini_topic_memory(custom_topic);
        custom_topic = NULL;
        goto fail;
    }

    topic_req = uxr_buffer_create_topic_ref(
        &custom_node->context->session,
        custom_node->context->reliable_output, custom_topic->topic_id,
        custom_node->participant_id, rmw_uxrce_profile_name, UXR_REPLACE);
#endif

    // Send the request and wait for response
    custom_topic->sync_with_agent = run_xrce_session(custom_node->context, topic_req);

    if (!custom_topic->sync_with_agent)
    {
        rmw_uxrce_fini_topic_memory(custom_topic);
        custom_topic = NULL;
        goto fail;
    }

fail:
    return(custom_topic);
}

rmw_ret_t destroy_topic(rmw_uxrce_topic_t* topic)
{
    rmw_ret_t         result_ret  = RMW_RET_OK;
    rmw_uxrce_node_t* custom_node = topic->owner_node;

    uint16_t delete_topic = uxr_buffer_delete_entity(
        &custom_node->context->session,
        *custom_node->context->creation_destroy_stream,
        topic->topic_id);

    if (!run_xrce_session(custom_node->context, delete_topic))
    {
        result_ret = RMW_RET_ERROR;
    }
    else
    {
        rmw_uxrce_fini_topic_memory(topic);
        result_ret = RMW_RET_OK;
    }
    return(result_ret);
}

size_t topic_count(rmw_uxrce_node_t* custom_node)
{
    size_t count = 0;
    rmw_uxrce_mempool_item_t* item = NULL;

    item = publisher_memory.allocateditems;
    while (item != NULL)
    {
        rmw_uxrce_publisher_t* custom_publisher = (rmw_uxrce_publisher_t*)item->data;
        item = item->next;
        if (custom_publisher->owner_node == custom_node && custom_publisher->topic != NULL)
        {
            count++;
        }
    }

    item = subscription_memory.allocateditems;
    while (item != NULL)
    {
        rmw_uxrce_subscription_t* custom_subscription = (rmw_uxrce_subscription_t*)item->data;
        item = item->next;
        if (custom_subscription->owner_node == custom_node && custom_subscription->topic != NULL)
        {
            count++;
        }
    }

    return(count);
}
