#include "rmw_publisher.h"

#include "micronode.h"
#include "rmw_node.h"
#include "utils.h"

#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rosidl_typesupport_micrortps_c/identifier.h>

rmw_publisher_t* create_publisher(const rmw_node_t* node, const rosidl_message_type_support_t* type_support,
                                  const char* topic_name, const rmw_qos_profile_t* qos_policies)
{
    (void)qos_policies;

    rmw_publisher_t* rmw_publisher           = (rmw_publisher_t*)rmw_allocate(sizeof(rmw_publisher_t));
    rmw_publisher->data                      = NULL;
    rmw_publisher->implementation_identifier = rmw_get_implementation_identifier();
    rmw_publisher->topic_name                = (const char*)(rmw_allocate(sizeof(char) * strlen(topic_name) + 1));
    if (!rmw_publisher->topic_name)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        rmw_free_and_null(rmw_publisher);
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
    MicroNode* micro_node = (MicroNode*)node->data;
    if (micro_node->num_publishers == MAX_PUBLISHERS     - 1)
    {
        RMW_SET_ERROR_MSG("Max number of publisher reached")
        rmw_free_and_null(rmw_publisher);
    }

    // TODO micro_rtps_id is duplicated in publisher_id and in publisher_gid.data
    PublisherInfo* publisher_info = &micro_node->publisher_info[micro_node->num_publishers++];
    rmw_publisher->data           = publisher_info;

    publisher_info->publisher_id                            = mr_object_id(0x01, MR_PUBLISHER_ID);
    publisher_info->typesupport_identifier                  = type_support->typesupport_identifier;
    publisher_info->publisher_gid.implementation_identifier = rmw_get_implementation_identifier();
    memset(publisher_info->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
    memcpy(publisher_info->publisher_gid.data, &publisher_info->publisher_id, sizeof(mrObjectId));

    const char* publisher_xml = "<publisher name=\"MyPublisher\">";
    uint16_t publisher_req =
        mr_write_configure_publisher_xml(&micro_node->session, reliable_output, publisher_info->publisher_id,
                                         micro_node->participant_id, publisher_xml, MR_REPLACE);

    publisher_info->topic_id = mr_object_id(0x01, MR_TOPIC_ID);
    const char* topic_xml    = "<dds><topic><name>HelloWorldTopic</name><dataType>HelloWorld</dataType></topic></dds>";
    uint16_t topic_req = mr_write_configure_topic_xml(&micro_node->session, reliable_output, publisher_info->topic_id,
                                                      micro_node->participant_id, topic_xml, MR_REPLACE);

    publisher_info->datawriter_id = mr_object_id(0x01, MR_DATAWRITER_ID);
    const char* datawriter_xml =
        "<profiles><publisher "
        "profile_name=\"default_xrce_publisher_profile\"><topic><kind>NO_KEY</kind><name>HelloWorldTopic</"
        "name><dataType>HelloWorld</dataType><historyQos><kind>KEEP_LAST</kind><depth>5</depth></"
        "historyQos><durability><kind>TRANSIENT_LOCAL</kind></durability></topic></publisher></profiles>";
    uint16_t datawriter_req =
        mr_write_configure_datawriter_xml(&micro_node->session, reliable_output, publisher_info->datawriter_id,
                                          publisher_info->publisher_id, datawriter_xml, MR_REPLACE);
    uint8_t status[3];
    uint16_t requests[] = {publisher_req, datawriter_req, topic_req};
    if (!mr_run_session_until_status(&micro_node->session, 1000, requests, status, 3))
    {
        rmw_free(rmw_publisher);
        rmw_publisher = NULL;
    }
    return rmw_publisher;
}