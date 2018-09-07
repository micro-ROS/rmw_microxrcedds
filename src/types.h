#ifndef RMW_MICRORTPS_TYPES_H_
#define RMW_MICRORTPS_TYPES_H_

#include "memory.h"
#include "rmw_macros.h"

#include <rmw/types.h>
#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_typesupport_micrortps_c/message_type_support.h"

#include <micrortps/client/client.h>
#include <microcdr/microcdr.h>

#include <stddef.h>


#define MAX_TRANSPORT_MTU 512
#define MAX_HISTORY 16
#define MAX_BUFFER_SIZE MAX_TRANSPORT_MTU* MAX_HISTORY

#define MAX_NODES 1
#define MAX_PUBLISHERS_X_NODE 10
#define MAX_SUBSCRIPTIONS_X_NODE 10

#define RMW_MICRORTPS_SUBSCRIBER_RAW_BUFFER_SIZE 500

typedef struct CustomSubscription
{
    bool in_use;
    mrObjectId subscriber_id;
    mrObjectId datareader_id;
    mrObjectId topic_id;
    rmw_gid_t subscription_gid;
    const char* typesupport_identifier;
    struct Item mem;

    const message_type_support_callbacks_t* type_support;

    struct 
    {
        uint8_t MemHead[RMW_MICRORTPS_SUBSCRIBER_RAW_BUFFER_SIZE];
        uint8_t * MemTail; /// \Note MemTail always points to the last non readable array data
        uint8_t * Write;
        uint8_t * Read;
        size_t RawDataSize; /// \Note Used to keep track of the DataSize type
    } TmpRawBuffer
} CustomSubscription;

typedef struct CustomPublisher
{
    bool in_use;
    mrObjectId publisher_id;
    mrObjectId datawriter_id;
    mrObjectId topic_id;
    rmw_gid_t publisher_gid;
    const char* typesupport_identifier;
    const rosidl_message_type_support_t* type_support;
    struct CustomNode* custom_node;
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

} CustomNode;

mrStreamId best_input;
mrStreamId reliable_input;
mrStreamId best_output;
mrStreamId reliable_output;

uint8_t input_reliable_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_best_effort_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_reliable_stream_buffer[MAX_BUFFER_SIZE];

void init_nodes_memory(struct MemPool* memory, CustomNode nodes[static MAX_NODES], size_t size);

#endif // !RMW_MICRORTPS_TYPES_H_