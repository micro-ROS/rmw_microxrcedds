#include "rmw_node.h"

#include "micronode.h"
#include "utils.h"

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

MicroNode node_info;

void on_status(mrSession* session, mrObjectId object_id, uint16_t request_id, uint8_t status, void* args)
{
    (void)session;
    (void)object_id;
    (void)request_id;
    (void)status;
    (void)args;
}

void on_topic(mrSession* session, mrObjectId object_id, uint16_t request_id, mrStreamId stream_id,
              struct MicroBuffer* serialization, void* args)
{
    (void)session;
    (void)object_id;
    (void)request_id;
    (void)stream_id;
    (void)serialization;
    (void)args;

    // ShapeType topic;
    // deserialize_ShapeType_topic(serialization, &topic);

    printf("Receiving... ");
    // print_ShapeType_topic(&topic);
}

void clear_node(rmw_node_t* node)
{
    MicroNode* micro_node = (MicroNode*)node->data;
    mr_delete_session(&micro_node->session);
    mr_close_udp_transport(&micro_node->udp);
    rmw_node_free_and_null_all(node);
}

rmw_node_t* create_node(const char* name, const char* namespace_, size_t domain_id)
{
    static const char* ip       = "127.0.0.1";
    static const uint16_t port  = 8888;
    static const uint32_t key   = 0xAABBCCDD;
    static const size_t history = 8;

    // TODO(Borja) Think how we are going to select transport to use
    if (!mr_init_udp_transport(&node_info.udp, ip, port))
    {
        RMW_SET_ERROR_MSG("Can not create an udp connection");
        return NULL;
    }
    printf("UDP mode => ip: %s - port: %hu\n", ip, port);

    mr_init_session(&node_info.session, &node_info.udp.comm, key);
    mr_set_topic_callback(&node_info.session, on_topic, NULL);
    mr_set_status_callback(&node_info.session, on_status, NULL);

    best_input      = mr_create_input_best_effort_stream(&node_info.session);
    reliable_input  = mr_create_input_reliable_stream(&node_info.session, input_reliable_stream_buffer,
                                                     node_info.udp.comm.mtu * history, history);
    best_output     = mr_create_output_best_effort_stream(&node_info.session, output_best_effort_stream_buffer,
                                                      node_info.udp.comm.mtu);
    reliable_output = mr_create_output_reliable_stream(&node_info.session, output_reliable_stream_buffer,
                                                       node_info.udp.comm.mtu * history, history);

    rmw_node_t* node_handle = NULL;
    node_handle             = rmw_node_allocate();
    if (!node_handle)
    {
        RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
        return NULL;
    }
    node_handle->implementation_identifier = rmw_get_implementation_identifier();
    node_handle->data                      = &node_info;
    node_handle->name                      = (const char*)(rmw_allocate(sizeof(char) * strlen(name) + 1));
    if (!node_handle->name)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        rmw_node_free_and_null(node_handle);
        return NULL;
    }
    memcpy((char*)node_handle->name, name, strlen(name) + 1);

    node_handle->namespace_ = rmw_allocate(sizeof(char) * strlen(namespace_) + 1);
    if (!node_handle->namespace_)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        rmw_free_and_null((char*)node_handle->name);
        rmw_node_free_and_null(node_handle);
        return NULL;
    }
    memcpy((char*)node_handle->namespace_, namespace_, strlen(namespace_) + 1);

    if (!mr_create_session(&node_info.session))
    {
        rmw_node_free_and_null_all(node_handle);
        return NULL;
    }

    // Create the Node participant. At this point a Node correspond with a Session with one participant.
    node_info.participant_id    = mr_object_id(1, MR_PARTICIPANT_ID);
    const char* participant_ref = "default participant";
    uint16_t participant_req    = mr_write_create_participant_ref(
        &node_info.session, reliable_output, node_info.participant_id, domain_id, participant_ref, MR_REPLACE);
    uint8_t status[1];
    uint16_t requests[] = {participant_req};

    if (!mr_run_session_until_status(&node_info.session, 1000, requests, status, 1))
    {
        mr_delete_session(&node_info.session);
        rmw_node_free_and_null_all(node_handle);
        return NULL;
    }

    return node_handle;
}

rmw_ret_t rmw_destroy_node(rmw_node_t* node)
{
    EPROS_PRINT_TRACE()
    rmw_ret_t result_ret = RMW_RET_OK;
    if (!node)
    {
        RMW_SET_ERROR_MSG("node handle is null");
        return RMW_RET_ERROR;
    }

    if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) == 0)
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
        return RMW_RET_ERROR;
    }

    if (!node->data)
    {
        RMW_SET_ERROR_MSG("node impl is null");
        return RMW_RET_ERROR;
    }

    clear_node(node);
    return result_ret;
}