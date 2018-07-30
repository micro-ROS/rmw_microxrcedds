#include "rmw_node.h"

#include "identifier.h"

#include <micrortps/client/client.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"

#define MAX_TRANSPORT_MTU 512
#define MAX_HISTORY 16
#define MAX_BUFFER_SIZE MAX_TRANSPORT_MTU* MAX_HISTORY

// Todo(borja): use static memory allocations with a fixed number of sessions/nodes.
typedef struct MicroNode
{
    mrUDPTransport udp;
    mrSession session;
} MicroNode;

MicroNode node_info;

void on_status(mrSession* session, mrObjectId object_id, uint16_t request_id, uint8_t status, void* args)
{
    (void)session;
    (void)object_id;
    (void)request_id;
    (void)args;    
}

void on_topic(mrSession* session, mrObjectId object_id, uint16_t request_id, mrStreamId stream_id, struct MicroBuffer* serialization, void* args)
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

rmw_node_t* create_node(const char* name, const char* namespace_)
{
    char* ip      = "127.0.0.1";
    uint16_t port = 8888;
    if (!mr_init_udp_transport(&node_info.udp, ip, port))
    {
        RMW_SET_ERROR_MSG("Can not create an udp connection");
        return RMW_RET_ERROR;
    }
    printf("UDP mode => ip: %s - port: %hu\n", ip, port);

    uint32_t key       = 0xAABBCCDD;
    size_t history     = 8;

    mr_init_session(&node_info.session, &node_info.udp.comm, key);
    mr_set_topic_callback(&node_info.session, on_topic, NULL);
    mr_set_status_callback(&node_info.session, on_status, NULL);

    uint8_t input_reliable_stream_buffer[MAX_BUFFER_SIZE];
    uint8_t output_best_effort_stream_buffer[MAX_BUFFER_SIZE];
    uint8_t output_reliable_stream_buffer[MAX_BUFFER_SIZE];

    (void)mr_create_input_best_effort_stream(&node_info.session);
    (void)mr_create_input_reliable_stream(&node_info.session, input_reliable_stream_buffer, node_info.udp.comm.mtu * history, history);
    (void)mr_create_output_best_effort_stream(&node_info.session, output_best_effort_stream_buffer, node_info.udp.comm.mtu);
    mrStreamId reliable_output =
        mr_create_output_reliable_stream(&node_info.session, output_reliable_stream_buffer, node_info.udp.comm.mtu * history, history);

    rmw_node_t * node_handle = NULL;
    node_handle = rmw_node_allocate();
    // node_handle = malloc(sizeof(rmw_node_t));
    if (!node_handle)
    {
        RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
        return NULL;
    }
    // node_handle->implementation_identifier = rmw_get_implementation_identifier();
    node_handle->data                      = NULL;//&node_info;
    node_handle->name                      = (const char*)(rmw_allocate(sizeof(char) * strlen(name) + 1));
    if (!node_handle->name)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        node_handle->namespace_ = NULL; // to avoid free on uninitialized memory
        return NULL;
    }
    memcpy(node_handle->name, name, strlen(name) + 1);

    node_handle->namespace_ = rmw_allocate(sizeof(char) * strlen(namespace_) + 1);
    if (!node_handle->namespace_)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        // goto fail;
    }
    memcpy(node_handle->namespace_, namespace_, strlen(namespace_) + 1);

    (void)mr_create_session(&node_info.session);

    mrObjectId participant_id = mr_object_id(1, MR_PARTICIPANT_ID);
    char* participant_ref = "default participant";
    (void)mr_write_create_participant_ref(&node_info.session, reliable_output, participant_id, participant_ref, 0);
    (void)mr_run_session_until_timeout(&node_info.session, 20);

    return node_handle;
  
    // fail:
    //   if (node_handle) {
    //     rmw_free(const_cast<char *>(node_handle->namespace_));
    //     node_handle->namespace_ = nullptr;
    //     rmw_free(const_cast<char *>(node_handle->name));
    //     node_handle->name = nullptr;
    //   }
    //   rmw_node_free(node_handle);
    //   delete node_impl;
    //   if (graph_guard_condition) {
    //     rmw_ret_t ret = rmw_destroy_guard_condition(graph_guard_condition);
    //     if (ret != RMW_RET_OK) {
    //       RCUTILS_LOG_ERROR_NAMED(
    //         "rmw_fastrtps_cpp",
    //         "failed to destroy guard condition during error handling")
    //     }
    //   }
    //   rmw_free(listener);
    //   if (participant) {
    //     Domain::removeParticipant(participant);
    //   }
}