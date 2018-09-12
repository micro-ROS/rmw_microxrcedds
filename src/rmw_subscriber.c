#include "rmw_subscriber.h"

#include "rmw_micrortps.h"
#include "types.h"
#include "utils.h"

#include <rmw/allocators.h>
#include <rmw/error_handling.h>

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
    // // TODO custom publisher info contining typesuppot pointer and id
    // const rosidl_message_type_support_t* message_type_support =
    //     get_message_typesupport_handle(type_support, rosidl_typesupport_micrortps_c__identifier);
    // if (!type_support)
    // {
    //     RMW_SET_ERROR_MSG("type support not from this implementation");
    //     return NULL;
    // }

    // TODO(Borja) Check NULL on node
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
            // TODO micro_rtps_id is duplicated in subscriber_id and in subscription_gid.data
            CustomSubscription* subscription_info                         = (CustomSubscription*)memory_node->data;
            subscription_info->subscriber_id                              = mr_object_id(micro_node->id_gen++, MR_SUBSCRIBER_ID);
            subscription_info->typesupport_identifier                     = type_support->typesupport_identifier;
            subscription_info->type_support                               = (message_type_support_callbacks_t* )type_support->data;
            subscription_info->subscription_gid.implementation_identifier = rmw_get_implementation_identifier();
            subscription_info->session                                    = &micro_node->session;
            subscription_info->owner_node                                 = micro_node;

            subscription_info->tmp_raw_buffer.write = subscription_info->tmp_raw_buffer.mem_head;
            subscription_info->tmp_raw_buffer.read = subscription_info->tmp_raw_buffer.mem_head;
            subscription_info->tmp_raw_buffer.mem_tail = &subscription_info->tmp_raw_buffer.mem_head[sizeof(subscription_info->tmp_raw_buffer.mem_head)];

            if (sizeof(mrObjectId) > RMW_GID_STORAGE_SIZE)
            {
                RMW_SET_ERROR_MSG("Max number of publisher reached")
            }
            else
            {
                memset(subscription_info->subscription_gid.data, 0, RMW_GID_STORAGE_SIZE);
                memcpy(subscription_info->subscription_gid.data, &subscription_info->subscriber_id, sizeof(mrObjectId));

                const char* subscriber_xml = "<subscriber name=\"MySubscriber\">";
                uint16_t subscriber_req    = mr_write_configure_subscriber_xml(
                    &micro_node->session, reliable_output, subscription_info->subscriber_id, micro_node->participant_id,
                    subscriber_xml, MR_REPLACE);

                subscription_info->topic_id = mr_object_id(micro_node->id_gen++, MR_TOPIC_ID);
                const char* topic_xml =
                    "<dds><topic><name>Int32MsgPubSubTopic</name><dataType>Int32Msg</dataType></topic></dds>";
                uint16_t topic_req =
                    mr_write_configure_topic_xml(&micro_node->session, reliable_output, subscription_info->topic_id,
                                                 micro_node->participant_id, topic_xml, MR_REPLACE);

                subscription_info->datareader_id = mr_object_id(micro_node->id_gen++, MR_DATAREADER_ID);
                const char* datareader_xml =
                    "<profiles><subscriber "
                    "profile_name=\"default_xrce_subscriber_profile\"><topic><kind>NO_KEY</kind><name>Int32MsgPubSubTopic</"
                    "name><dataType>Int32Msg</dataType><historyQos><kind>KEEP_LAST</kind><depth>5</depth></"
                    "historyQos><durability><kind>TRANSIENT_LOCAL</kind></durability></topic></subscriber></profiles>";
                uint16_t datareader_req = mr_write_configure_datareader_xml(
                    &micro_node->session, reliable_output, subscription_info->datareader_id,
                    subscription_info->subscriber_id, datareader_xml, MR_REPLACE);

                rmw_subscriber->data = subscription_info;
                uint8_t status[3];
                uint16_t requests[] = {subscriber_req, datareader_req, topic_req};
                if (!mr_run_session_until_status(&micro_node->session, 1000, requests, status, 3))
                {
                    RMW_SET_ERROR_MSG("Issues creating micro RTPS entities");
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
            mr_write_delete_entity(&micro_node->session, reliable_output, subscripion_info->datareader_id);
        int delete_topic = mr_write_delete_entity(&micro_node->session, reliable_output, subscripion_info->topic_id);
        int delete_subscriber =
            mr_write_delete_entity(&micro_node->session, reliable_output, subscripion_info->subscriber_id);

        uint8_t status[3];
        uint16_t requests[] = {delete_datareader, delete_topic, delete_subscriber};
        if (!mr_run_session_until_status(&micro_node->session, 1000, requests, status, 3))
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