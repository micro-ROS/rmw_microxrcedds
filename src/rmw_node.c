#include "rmw_node.h"

#include "types.h"
#include "utils.h"

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

static struct MemPool node_memory;
static CustomNode custom_nodes[MAX_NODES];

void init_rmw_node()
{
    init_nodes_memory(&node_memory, custom_nodes, MAX_NODES);
}

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
    (void)request_id;
    (void)stream_id;

    // Get node pointer
    CustomNode* node = (CustomNode*)args;

    // Search subcription
    struct Item* subscription_item = node->subscription_mem.allocateditems;
    while (true)
    {
        // Check if end of stack
        if (subscription_item == NULL)
        {
            return;
        }

        // Compare id
        if (memcmp(&(((CustomSubscription*)subscription_item->data)->datareader_id), &object_id, sizeof(mrObjectId)) ==
            0)
        {
            break;
        }

        // Next subcription of the stack
        subscription_item = subscription_item->next;
    }

    // get buffer size
    CustomSubscription* subscription       = ((CustomSubscription*)subscription_item->data);
    subscription->TmpRawBuffer.RawDataSize = micro_buffer_remaining(serialization);
    if (subscription->TmpRawBuffer.RawDataSize == 0)
    {
        return;
    }

    // get needed bytes space
    size_t NeededSpace = sizeof(serialization->endianness) + sizeof(subscription->TmpRawBuffer.RawDataSize) +
                         subscription->TmpRawBuffer.RawDataSize;

    // check if there is enogh space at the end of the tmp raw buffer
    if ((&(subscription->TmpRawBuffer.MemHead[sizeof(subscription->TmpRawBuffer.MemHead)]) -
         subscription->TmpRawBuffer.Write) < NeededSpace)
    {

        // check if there is enogh space at the begining of the tmp raw buffer
        if ((subscription->TmpRawBuffer.Read - subscription->TmpRawBuffer.MemHead) < NeededSpace)
        {
            // not enough space to store the data
            RMW_SET_ERROR_MSG("Incomming data lost due to not enough storage memory");
            return;
        }

        // Move tail pointer to the bigining and relocate tail
        subscription->TmpRawBuffer.MemTail = subscription->TmpRawBuffer.Write;
        subscription->TmpRawBuffer.Write   = subscription->TmpRawBuffer.MemHead;
    }

    // Save microbuffer for a future processing (Endianness + subscription->TmpRawBuffer.RawDataSize + MicroBufferData)
    memcpy(subscription->TmpRawBuffer.Write, &serialization->endianness, sizeof(serialization->endianness));
    subscription->TmpRawBuffer.Write += sizeof(serialization->endianness);

    memcpy(subscription->TmpRawBuffer.Write, &subscription->TmpRawBuffer.RawDataSize,
           sizeof(subscription->TmpRawBuffer.RawDataSize));
    subscription->TmpRawBuffer.Write += sizeof(subscription->TmpRawBuffer.RawDataSize);

    memcpy(subscription->TmpRawBuffer.Write, serialization->iterator, subscription->TmpRawBuffer.RawDataSize);
    subscription->TmpRawBuffer.Write += subscription->TmpRawBuffer.RawDataSize;

    return;
}

void clear_node(rmw_node_t* node)
{
    CustomNode* micro_node = (CustomNode*)node->data;
    // TODO make sure that session deletion deletes participant and related entities.
    mr_delete_session(&micro_node->session);
    mr_close_udp_transport(&micro_node->udp);
    rmw_node_delete(node);
}

rmw_node_t* create_node(const char* name, const char* namespace_, size_t domain_id)
{
    static const char* ip       = "127.0.0.1";
    static const uint16_t port  = 8888;
    static const uint32_t key   = 0xAABBCCDD;
    static const size_t history = 8;

    struct Item* memory_node = get_memory(&node_memory);
    if (!memory_node)
    {
        RMW_SET_ERROR_MSG("Not available memory node");
        return NULL;
    }

    CustomNode* node_info = (CustomNode*)memory_node->data;
    // TODO(Borja) Think how we are going to select transport to use
    if (!mr_init_udp_transport(&node_info->udp, ip, port))
    {
        RMW_SET_ERROR_MSG("Can not create an udp connection");
        return NULL;
    }
    printf("UDP mode => ip: %s - port: %hu\n", ip, port);

    mr_init_session(&node_info->session, &node_info->udp.comm, key);
    mr_set_topic_callback(&node_info->session, on_topic, node_info);
    mr_set_status_callback(&node_info->session, on_status, NULL);

    best_input      = mr_create_input_best_effort_stream(&node_info->session);
    reliable_input  = mr_create_input_reliable_stream(&node_info->session, input_reliable_stream_buffer,
                                                     node_info->udp.comm.mtu * history, history);
    best_output     = mr_create_output_best_effort_stream(&node_info->session, output_best_effort_stream_buffer,
                                                      node_info->udp.comm.mtu);
    reliable_output = mr_create_output_reliable_stream(&node_info->session, output_reliable_stream_buffer,
                                                       node_info->udp.comm.mtu * history, history);

    rmw_node_t* node_handle = NULL;
    node_handle             = rmw_node_allocate();
    if (!node_handle)
    {
        RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
        return NULL;
    }
    node_handle->implementation_identifier = rmw_get_implementation_identifier();
    node_handle->data                      = node_info;
    node_handle->name                      = (const char*)(rmw_allocate(sizeof(char) * strlen(name) + 1));
    if (!node_handle->name)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        mr_close_udp_transport(&node_info->udp);
        rmw_node_delete(node_handle);
        return NULL;
    }
    memcpy((char*)node_handle->name, name, strlen(name) + 1);

    node_handle->namespace_ = rmw_allocate(sizeof(char) * strlen(namespace_) + 1);
    if (!node_handle->namespace_)
    {
        RMW_SET_ERROR_MSG("failed to allocate memory");
        mr_close_udp_transport(&node_info->udp);
        rmw_node_delete(node_handle);
        return NULL;
    }
    memcpy((char*)node_handle->namespace_, namespace_, strlen(namespace_) + 1);

    if (!mr_create_session(&node_info->session))
    {
        mr_close_udp_transport(&node_info->udp);
        rmw_node_delete(node_handle);
        return NULL;
    }

    // Create the Node participant. At this point a Node correspond with a Session with one participant.
    node_info->participant_id   = mr_object_id(1, MR_PARTICIPANT_ID);
    const char* participant_ref = "default participant";
    uint16_t participant_req    = mr_write_create_participant_ref(
        &node_info->session, reliable_output, node_info->participant_id, domain_id, participant_ref, MR_REPLACE);
    uint8_t status[1];
    uint16_t requests[] = {participant_req};

    if (!mr_run_session_until_status(&node_info->session, 1000, requests, status, 1))
    {
        mr_delete_session(&node_info->session);
        mr_close_udp_transport(&node_info->udp);
        rmw_node_delete(node_handle);
        RMW_SET_ERROR_MSG("Issues creating micro RTPS entities");
        return NULL;
    }

    // TODO create utils methods to handle publishers array.
    customnode_clear(node_info);

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

    if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0)
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