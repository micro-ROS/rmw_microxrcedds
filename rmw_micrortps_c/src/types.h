#ifndef RMW_MICRORTPS_TYPES_H_
#define RMW_MICRORTPS_TYPES_H_

#include "memory.h"

#include "config.h"

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_typesupport_micrortps_c/message_type_support.h"
#include <rmw/types.h>

#include <microcdr/microcdr.h>
#include <micrortps/client/client.h>

#include <stddef.h>

typedef struct CustomSubscription
{
    mrObjectId subscriber_id;
    mrObjectId datareader_id;
    mrObjectId topic_id;
    rmw_gid_t subscription_gid;
    const message_type_support_callbacks_t* type_support_callbacks;
    struct CustomNode* owner_node;
    mrSession* session;

    struct
    {
        uint8_t mem_head[RMW_MICRORTPS_SUBSCRIBER_RAW_BUFFER_SIZE];
        uint8_t* mem_tail; /// \Note MemTail always points to the last non readable array data
        uint8_t* write;
        uint8_t* read;
        size_t raw_data_size; /// \Note Used to keep track of the DataSize type
    } tmp_raw_buffer;

    bool waiting_for_response;
    uint16_t suncription_request;

    struct Item mem;
} CustomSubscription;

typedef struct CustomPublisher
{
    mrObjectId publisher_id;
    mrObjectId datawriter_id;
    mrObjectId topic_id;
    rmw_gid_t publisher_gid;
    const message_type_support_callbacks_t* type_support_callbacks;
    mrSession* session;
    struct Item mem;

    struct CustomNode* owner_node;

} CustomPublisher;

typedef struct CustomNode
{
#ifdef MICRO_RTPS_SERIAL
    mrSerialTransport transport;
#elif defined(MICRO_RTPS_UDP)
    mrUDPTransport transport;
#endif
    mrSession session;
    mrObjectId participant_id;
    struct MemPool publisher_mem;
    struct MemPool subscription_mem;
    struct Item mem;

    CustomPublisher publisher_info[MAX_PUBLISHERS_X_NODE];
    CustomSubscription subscription_info[MAX_SUBSCRIPTIONS_X_NODE];

    bool on_subcription;

    mrStreamId reliable_input;
    mrStreamId reliable_output;

    uint8_t input_reliable_stream_buffer[MAX_BUFFER_SIZE];
    uint8_t output_reliable_stream_buffer[MAX_BUFFER_SIZE];

    uint8_t miscellaneous_temp_buffer[MAX_TRANSPORT_MTU];

    uint16_t id_gen;

} CustomNode;

void init_nodes_memory(struct MemPool* memory, CustomNode nodes[MAX_NODES], size_t size);

#endif // !RMW_MICRORTPS_TYPES_H_
