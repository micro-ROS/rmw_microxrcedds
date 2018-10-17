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

#include "rmw_publisher.h"

#include "rmw_microxrcedds.h"
#include "rmw_node.h"
#include "types.h"
#include "utils.h"

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#include <rosidl_typesupport_microxrcedds_c/message_type_support.h>


rmw_publisher_t* create_publisher(const rmw_node_t* node, const rosidl_message_type_support_t* type_support,
                                  const char* topic_name, const rmw_qos_profile_t* qos_policies)
{
    bool success = false;

    rmw_publisher_t* rmw_publisher           = (rmw_publisher_t*)rmw_allocate(sizeof(rmw_publisher_t));
    rmw_publisher->data                      = NULL;
    rmw_publisher->implementation_identifier = rmw_get_implementation_identifier();
    rmw_publisher->topic_name                = (const char*)(rmw_allocate(sizeof(char) * strlen(topic_name) + 1));
    if (!rmw_publisher->topic_name)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        return NULL;
    }
    else
    {
        CustomNode* micro_node   = (CustomNode*)node->data;
        struct Item* memory_node = get_memory(&micro_node->publisher_mem);
        if (!memory_node)
        {
            RMW_SET_ERROR_MSG("Not available memory node");
            return NULL;
        }
        else
        {
            // TODO micro_xrcedds_id is duplicated in publisher_id and in publisher_gid.data
            CustomPublisher* publisher_info = (CustomPublisher*)memory_node->data;
            publisher_info->publisher_id    = uxr_object_id(micro_node->id_gen++, UXR_PUBLISHER_ID);
            publisher_info->owner_node      = micro_node;
            publisher_info->publisher_gid.implementation_identifier = rmw_get_implementation_identifier();
            publisher_info->session                                 = &micro_node->session;
            publisher_info->type_support_callbacks = (const message_type_support_callbacks_t*)type_support->data;
            if (publisher_info->type_support_callbacks == NULL)
            {
                RMW_SET_ERROR_MSG("Typesupport data is null");
                return NULL;
            }
            else if (strcmp(type_support->typesupport_identifier, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE) != 0)
            {
                RMW_SET_ERROR_MSG("type support not from this implementation");
                return NULL;
            }
            else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE)
            {
                RMW_SET_ERROR_MSG("Not enough memory for impl ids");
                return NULL;
            }
            else
            {
                memset(publisher_info->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
                memcpy(publisher_info->publisher_gid.data, &publisher_info->publisher_id, sizeof(uxrObjectId));

                uint16_t publisher_req;
#ifdef MICRO_XRCEDDS_USE_XML
                char publisher_name[20];
                generate_name(&publisher_info->publisher_id, publisher_name, sizeof(publisher_name));
                char xml_buffer[400];
                if (!build_publisher_xml(publisher_name, xml_buffer, sizeof(xml_buffer)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for publisher creation");
                    return NULL;
                }
                publisher_req = uxr_buffer_create_publisher_xml(publisher_info->session, micro_node->reliable_output,
                                                                 publisher_info->publisher_id,
                                                                 micro_node->participant_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
                // Publisher by reference does not make sense in current micro XRCE-DDS implementation.
                publisher_req = uxr_buffer_create_publisher_xml(publisher_info->session, micro_node->reliable_output,
                                                                 publisher_info->publisher_id,
                                                                 micro_node->participant_id, "", UXR_REPLACE);
#endif
                publisher_info->topic_id = uxr_object_id(micro_node->id_gen++, UXR_TOPIC_ID);

                uint16_t topic_req;
#ifdef MICRO_XRCEDDS_USE_XML
                if (!build_topic_xml(topic_name, publisher_info->type_support_callbacks, qos_policies, xml_buffer,
                                     sizeof(xml_buffer)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for publisher creation");
                    return NULL;
                }

                topic_req = uxr_buffer_create_topic_xml(publisher_info->session, micro_node->reliable_output,
                                                         publisher_info->topic_id, micro_node->participant_id,
                                                         xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
                char profile_name[64];
                if (!build_topic_profile(topic_name, profile_name, sizeof(profile_name)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
                    return NULL;
                }
                topic_req = uxr_buffer_create_topic_ref(publisher_info->session, micro_node->reliable_output,
                                                      publisher_info->topic_id, micro_node->participant_id,
                                                      profile_name, UXR_REPLACE);
#endif
                publisher_info->datawriter_id = uxr_object_id(micro_node->id_gen++, UXR_DATAWRITER_ID);

                uint16_t datawriter_req;
#ifdef MICRO_XRCEDDS_USE_XML
                if (!build_datawriter_xml(topic_name, publisher_info->type_support_callbacks, qos_policies, xml_buffer,
                                          sizeof(xml_buffer)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for publisher creation");
                    return NULL;
                }

                datawriter_req = uxr_buffer_create_datawriter_xml(
                    publisher_info->session, micro_node->reliable_output, publisher_info->datawriter_id,
                    publisher_info->publisher_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
                if (!build_datawriter_profile(topic_name, profile_name, sizeof(profile_name)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
                    return NULL;
                }

                datawriter_req = uxr_buffer_create_datawriter_ref(publisher_info->session, micro_node->reliable_output,
                                                                publisher_info->datawriter_id,
                                                                publisher_info->publisher_id, profile_name, UXR_REPLACE);
#endif
                rmw_publisher->data = publisher_info;
                uint16_t requests[] = {publisher_req, datawriter_req, topic_req};
                uint8_t status[sizeof(requests) / 2];
                if (!uxr_run_session_until_all_status(publisher_info->session, 1000, requests, status, 3))
                {
                    RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
                }
                else
                {
                    success = true;
                }
            }
        }
    }

    if (!success)
    {
        rmw_publisher_delete(rmw_publisher);
    }
    return rmw_publisher;
}

rmw_ret_t rmw_destroy_publisher(rmw_node_t* node, rmw_publisher_t* publisher)
{
    EPROS_PRINT_TRACE()
    rmw_ret_t result_ret = RMW_RET_OK;
    if (!node)
    {
        RMW_SET_ERROR_MSG("node handle is null");
        result_ret = RMW_RET_ERROR;
    }
    else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
        result_ret = RMW_RET_ERROR;
    }
    else if (!node->data)
    {
        RMW_SET_ERROR_MSG("node imp is null");
        result_ret = RMW_RET_ERROR;
    }
    else if (!publisher)
    {
        RMW_SET_ERROR_MSG("publisher handle is null");
        result_ret = RMW_RET_ERROR;
    }
    else if (strcmp(publisher->implementation_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("publisher handle not from this implementation");
        result_ret = RMW_RET_ERROR;
    }
    else if (!publisher->data)
    {
        RMW_SET_ERROR_MSG("publisher imp is null");
        result_ret = RMW_RET_ERROR;
    }
    else
    {
        CustomPublisher* publisher_info = (CustomPublisher*)publisher->data;

        int delete_writer = uxr_buffer_delete_entity(publisher_info->session, publisher_info->owner_node->reliable_output,
                                                   publisher_info->datawriter_id);
        int delete_topic  = uxr_buffer_delete_entity(publisher_info->session, publisher_info->owner_node->reliable_output,
                                                  publisher_info->topic_id);
        int delete_publisher = uxr_buffer_delete_entity(
            publisher_info->session, publisher_info->owner_node->reliable_output, publisher_info->publisher_id);

        uint8_t status[3];
        uint16_t requests[] = {delete_writer, delete_topic, delete_publisher};
        if (!uxr_run_session_until_all_status(publisher_info->session, 1000, requests, status, 3))
        {
            RMW_SET_ERROR_MSG("unable to remove publisher from the server");
            result_ret = RMW_RET_ERROR;
        }
        else
        {
            rmw_publisher_delete(publisher);
            result_ret = RMW_RET_OK;
        }
    }

    return result_ret;
}

rmw_ret_t rmw_publish(const rmw_publisher_t* publisher, const void* ros_message)
{
    EPROS_PRINT_TRACE()
    rmw_ret_t ret = RMW_RET_OK;
    if (!publisher)
    {
        RMW_SET_ERROR_MSG("publisher pointer is null");
        ret = RMW_RET_ERROR;
    }
    else if (!ros_message)
    {
        RMW_SET_ERROR_MSG("ros_message pointer is null");
        ret = RMW_RET_ERROR;
    }
    if (strcmp(publisher->implementation_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("publisher handle not from this implementation");
        ret = RMW_RET_ERROR;
    }
    else if (!publisher->data)
    {
        RMW_SET_ERROR_MSG("publisher imp is null");
        ret = RMW_RET_ERROR;
    }
    else
    {
        CustomPublisher* publisher_info                   = (CustomPublisher*)publisher->data;
        const message_type_support_callbacks_t* functions = publisher_info->type_support_callbacks;
        bool written                                      = true;
        uint32_t topic_length                             = functions->get_serialized_size(ros_message);
        uint32_t payload_length                           = 0;
        payload_length                                    = (uint16_t)(payload_length + 4); // request_id + object_id
        payload_length                                    = (uint16_t)(payload_length + 4); // request_id + object_id

        ucdrBuffer mb;
        if(uxr_prepare_output_stream(publisher_info->session, publisher_info->owner_node->reliable_output, publisher_info->datawriter_id, &mb, topic_length))
        {
            ucdrBuffer mb_topic;
            ucdr_init_buffer(&mb_topic, mb.iterator, topic_length);
            written &= functions->cdr_serialize(ros_message, &mb_topic);

            written &= uxr_run_session_until_confirm_delivery(publisher_info->session, 1000);
        }
        if (!written)
        {
            RMW_SET_ERROR_MSG("error publishing message");
            ret = RMW_RET_ERROR;
        }
        
    }
    return ret;
}






        //ucdrBuffer mb;
        //uint32_t topic_size = HelloWorld_size_of_topic(&topic, 0);
        //uxr_prepare_output_stream(&session, reliable_out, datawriter_id, &mb, topic_size);
        //HelloWorld_serialize_topic(&mb, &topic);

        //connected = uxr_run_session_time(&session, 1000);