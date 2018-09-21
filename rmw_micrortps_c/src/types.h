#ifndef RMW_MICRORTPS_TYPES_H_
#define RMW_MICRORTPS_TYPES_H_

#include "memory.h"

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_typesupport_micrortps_c/message_type_support.h"
#include <rmw/types.h>

#include <microcdr/microcdr.h>
#include <micrortps/client/client.h>

#include <stddef.h>

#define MAX_TRANSPORT_MTU 128
#define MAX_HISTORY 4
#define MAX_BUFFER_SIZE MAX_TRANSPORT_MTU * MAX_HISTORY

#define MAX_NODES 1
#define MAX_PUBLISHERS_X_NODE 4
#define MAX_SUBSCRIPTIONS_X_NODE 4
#define RMW_MICRORTPS_SUBSCRIBER_RAW_BUFFER_SIZE MAX_TRANSPORT_MTU

#define RMW_TOPIC_NAME_MAX_NAME_LENGTH 50

#ifndef USE_REFS
#ifndef USE_XML_REP
#define USE_XML_REP
#endif
#endif

// typedef struct message_type_support_callbacks_t message_type_support_callbacks_t;

typedef struct CustomSubscription
{
    mrObjectId subscriber_id;
    mrObjectId datareader_id;
    mrObjectId topic_id;
    rmw_gid_t subscription_gid;
    const message_type_support_callbacks_t* type_support;
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

    struct Item mem;
} CustomSubscription;

typedef struct CustomPublisher
{
    mrObjectId publisher_id;
    mrObjectId datawriter_id;
    mrObjectId topic_id;
    rmw_gid_t publisher_gid;
    const message_type_support_callbacks_t* type_support;
    mrSession* session;
    struct Item mem;
} CustomPublisher;

typedef struct CustomNode
{
    mrUDPTransport udp;
    mrSession session;
    mrObjectId participant_id;
    struct MemPool publisher_mem;
    struct MemPool subscription_mem;
    struct Item mem;

    CustomPublisher publisher_info[MAX_PUBLISHERS_X_NODE];
    CustomSubscription subscription_info[MAX_SUBSCRIPTIONS_X_NODE];

    uint8_t read_subscriptions_status[MAX_SUBSCRIPTIONS_X_NODE];
    uint16_t read_subscriptions_requests[MAX_SUBSCRIPTIONS_X_NODE];

    uint16_t id_gen;

} CustomNode;

mrStreamId best_input;
mrStreamId reliable_input;
mrStreamId best_output;
mrStreamId reliable_output;

uint8_t input_reliable_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_best_effort_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_reliable_stream_buffer[MAX_BUFFER_SIZE];

void init_nodes_memory(struct MemPool* memory, CustomNode nodes[MAX_NODES], size_t size);

#endif // !RMW_MICRORTPS_TYPES_H_
