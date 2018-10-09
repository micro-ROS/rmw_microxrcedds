#include "rmw_subscriber.h"

#include "rmw_microxrcedds.h"
#include "types.h"
#include "utils.h"

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rosidl_typesupport_microxrcedds_c/identifier.h>

rmw_subscription_t* create_subscriber(const rmw_node_t* node, const rosidl_message_type_support_t* type_support,
                                      const char* topic_name, const rmw_qos_profile_t* qos_policies,
                                      bool ignore_local_publications)
{
    bool success = false;
    (void)qos_policies;
    (void)ignore_local_publications;

    rmw_subscription_t* rmw_subscriber        = (rmw_subscription_t*)rmw_allocate(sizeof(rmw_subscription_t));
    rmw_subscriber->data                      = NULL;
    rmw_subscriber->implementation_identifier = rmw_get_implementation_identifier();
    rmw_subscriber->topic_name                = (const char*)(rmw_allocate(sizeof(char) * strlen(topic_name) + 1));
    if (!rmw_subscriber->topic_name)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
    }
    else
    {

        CustomNode* micro_node   = (CustomNode*)node->data;
        struct Item* memory_node = get_memory(&micro_node->subscription_mem);
        if (!memory_node)
        {
            RMW_SET_ERROR_MSG("Not available memory node");
            return NULL;
        }
        else
        {
            // TODO micro_xrcedds_id is duplicated in subscriber_id and in subscription_gid.data
            CustomSubscription* subscription_info = (CustomSubscription*)memory_node->data;
            subscription_info->subscriber_id      = uxr_object_id(micro_node->id_gen++, UXR_SUBSCRIBER_ID);
            subscription_info->subscription_gid.implementation_identifier = rmw_get_implementation_identifier();
            subscription_info->session                                    = &micro_node->session;
            subscription_info->owner_node                                 = micro_node;

            subscription_info->waiting_for_response = false;

            subscription_info->type_support_callbacks =
                get_message_typesupport_handle(type_support, rosidl_typesupport_microxrcedds_c__identifier)->data;
            if (!subscription_info->type_support_callbacks)
            {
                RMW_SET_ERROR_MSG("type support not from this implementation");
            }
            else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE)
            {
                RMW_SET_ERROR_MSG("Max number of publisher reached")
            }
            else
            {
                memset(subscription_info->subscription_gid.data, 0, RMW_GID_STORAGE_SIZE);
                memcpy(subscription_info->subscription_gid.data, &subscription_info->subscriber_id, sizeof(uxrObjectId));

                uint16_t subscriber_req;
#ifdef MICRO_XRCEDDS_USE_XML
                char subscriber_name[20];
                generate_name(&subscription_info->subscriber_id, subscriber_name, sizeof(subscriber_name));
                char xml_buffer[512];
                if (!build_subscriber_xml(subscriber_name, xml_buffer, sizeof(xml_buffer)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
                    return NULL;
                }
                subscriber_req = uxr_write_configure_subscriber_xml(&micro_node->session, micro_node->reliable_output,
                                                                   subscription_info->subscriber_id,
                                                                   micro_node->participant_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
                // Publisher by reference does not make sense in current micro XRCE-DDS implementation.
                subscriber_req = uxr_write_configure_subscriber_xml(&micro_node->session, micro_node->reliable_output,
                                                                   subscription_info->subscriber_id,
                                                                   micro_node->participant_id, "", UXR_REPLACE);
#endif
                subscription_info->topic_id = uxr_object_id(micro_node->id_gen++, UXR_TOPIC_ID);

                uint16_t topic_req;
#ifdef MICRO_XRCEDDS_USE_XML

                if (!build_topic_xml(topic_name, subscription_info->type_support_callbacks, qos_policies, xml_buffer,
                                     sizeof(xml_buffer)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
                    return NULL;
                }

                topic_req = uxr_write_configure_topic_xml(&micro_node->session, micro_node->reliable_output,
                                                         subscription_info->topic_id, micro_node->participant_id,
                                                         xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
                char profile_name[64];
                if (!build_topic_profile(topic_name, profile_name, sizeof(profile_name)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
                    return NULL;
                }
                topic_req = uxr_write_create_topic_ref(&micro_node->session, micro_node->reliable_output,
                                                      subscription_info->topic_id, micro_node->participant_id,
                                                      profile_name, UXR_REPLACE);
#endif
                subscription_info->datareader_id = uxr_object_id(micro_node->id_gen++, UXR_DATAREADER_ID);

                uint16_t datareader_req;
#ifdef MICRO_XRCEDDS_USE_XML
                if (!build_datareader_xml(topic_name, subscription_info->type_support_callbacks, qos_policies, xml_buffer,
                                          sizeof(xml_buffer)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
                    return NULL;
                }

                datareader_req = uxr_write_configure_datareader_xml(
                    &micro_node->session, micro_node->reliable_output, subscription_info->datareader_id,
                    subscription_info->subscriber_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
                if (!build_datareader_profile(topic_name, profile_name, sizeof(profile_name)))
                {
                    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
                    return NULL;
                }

                datareader_req = uxr_write_create_datareader_ref(
                    &micro_node->session, micro_node->reliable_output, subscription_info->datareader_id,
                    subscription_info->subscriber_id, profile_name, UXR_REPLACE);
#endif
                rmw_subscriber->data = subscription_info;
                uint8_t status[3];
                uint16_t requests[] = {subscriber_req, topic_req, datareader_req};
                if (!uxr_run_session_until_all_status(&micro_node->session, 1000, requests, status, 3))
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
        rmw_subscription_delete(rmw_subscriber);
    }
    return rmw_subscriber;
}

rmw_ret_t rmw_destroy_subscription(rmw_node_t* node, rmw_subscription_t* subscription)
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
    else if (!subscription)
    {
        RMW_SET_ERROR_MSG("subscription handle is null");
        result_ret = RMW_RET_ERROR;
    }
    else if (strcmp(subscription->implementation_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("subscription handle not from this implementation");
        result_ret = RMW_RET_ERROR;
    }
    else if (!subscription->data)
    {
        RMW_SET_ERROR_MSG("subscription imp is null");
        result_ret = RMW_RET_ERROR;
    }
    else
    {
        CustomNode* micro_node               = (CustomNode*)node->data;
        CustomSubscription* subscripion_info = (CustomSubscription*)subscription->data;
        int delete_datareader =
            uxr_write_delete_entity(&micro_node->session, micro_node->reliable_output, subscripion_info->datareader_id);
        int delete_topic =
            uxr_write_delete_entity(&micro_node->session, micro_node->reliable_output, subscripion_info->topic_id);
        int delete_subscriber =
            uxr_write_delete_entity(&micro_node->session, micro_node->reliable_output, subscripion_info->subscriber_id);

        uint8_t status[3];
        uint16_t requests[] = {delete_datareader, delete_topic, delete_subscriber};
        if (!uxr_run_session_until_all_status(&micro_node->session, 1000, requests, status, 3))
        {
            RMW_SET_ERROR_MSG("unable to remove publisher from the server");
            result_ret = RMW_RET_ERROR;
        }
        else
        {
            rmw_subscription_delete(subscription);
            result_ret = RMW_RET_OK;
        }
    }

    return result_ret;
}
