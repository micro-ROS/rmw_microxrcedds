#include "rmw_node.h"

#include "identifier.h"

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"

#define MAX_TRANSPORT_MTU 512
#define MAX_HISTORY 16
#define MAX_BUFFER_SIZE MAX_TRANSPORT_MTU* MAX_HISTORY

MicroNode node_info;
char* ip       = "127.0.0.1";
uint16_t port  = 8888;
uint32_t key   = 0xAABBCCDD;
size_t history = 8;

uint8_t input_reliable_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_best_effort_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_reliable_stream_buffer[MAX_BUFFER_SIZE];

void on_status(mrSession* session, mrObjectId object_id, uint16_t request_id, uint8_t status, void* args)
{
    (void)session;
    (void)object_id;
    (void)request_id;
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

void rmw_free_and_null(void* rmw_allocated_ptr)
{
    rmw_free(rmw_allocated_ptr);
    rmw_allocated_ptr = NULL;
}

void rmw_node_free_and_null(rmw_node_t* node)
{
    rmw_node_free(node);
    node = NULL;
}

rmw_node_t* create_node(const char* name, const char* namespace_, size_t domain_id)
{
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
        node_handle->name = NULL; // to avoid free on uninitialized memory
        rmw_node_free_and_null(node_handle);
        return NULL;
    }
    memcpy(node_handle->name, name, strlen(name) + 1);

    node_handle->namespace_ = rmw_allocate(sizeof(char) * strlen(namespace_) + 1);
    if (!node_handle->namespace_)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        node_handle->namespace_ = NULL;
        rmw_free_and_null(node_handle->name);
        rmw_node_free_and_null(node_handle);
        return NULL;
    }
    memcpy(node_handle->namespace_, namespace_, strlen(namespace_) + 1);

    if (!mr_create_session(&node_info.session))
    {
        rmw_free_and_null(node_handle->namespace_);
        rmw_free_and_null(node_handle->name);
        rmw_node_free_and_null(node_handle);
        return NULL;
    }

    // Create the Node participant. At this point a Node correspond with a Session with one participant.
    node_info.participant_id = mr_object_id(1, MR_PARTICIPANT_ID);
    char* participant_ref    = "default participant";
    uint16_t participant_req = mr_write_create_participant_ref(&node_info.session, reliable_output,
                                                               node_info.participant_id, domain_id, participant_ref, MR_REPLACE);
    uint8_t status[1];
    uint16_t requests[] = {participant_req};
    if (!mr_run_session_until_status(&node_info.session, 1000, requests, status, 1))
    {
        mr_delete_session(&node_info.session);
        rmw_free_and_null(node_handle->namespace_);
        rmw_free_and_null(node_handle->name);
        rmw_node_free_and_null(node_handle);
    }

    return node_handle;
}