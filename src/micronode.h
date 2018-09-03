#ifndef MICRONODE_H_
#define MICRONODE_H_

#include <micrortps/client/client.h>

#include <rmw/types.h>

#include <stddef.h>

#define MAX_TRANSPORT_MTU 512
#define MAX_HISTORY 16
#define MAX_BUFFER_SIZE MAX_TRANSPORT_MTU* MAX_HISTORY

#define MAX_PUBLISHERS 10
#define MAX_SUBSCRIPTIONS 10

// TODO(Borja): use static memory allocations with a fixed number of sessions/nodes.
typedef struct
{
    bool in_use;
    mrObjectId publisher_id;
    mrObjectId datawriter_id;
    mrObjectId topic_id;
    rmw_gid_t publisher_gid;
    const char* typesupport_identifier;
} PublisherInfo;

typedef struct
{
    bool in_use;
    mrObjectId subscriber_id;
    mrObjectId datareader_id;
    mrObjectId topic_id;
    rmw_gid_t subscription_gid;
    const char* typesupport_identifier;
} SubscriptionInfo;

typedef struct
{
    mrUDPTransport udp;
    mrSession session;
    mrObjectId participant_id;
    PublisherInfo publisher_info[MAX_PUBLISHERS];
    size_t num_publishers;
    SubscriptionInfo subscription_info[MAX_SUBSCRIPTIONS];
    size_t num_subscriptions;
} MicroNode;

mrStreamId best_input;
mrStreamId reliable_input;
mrStreamId best_output;
mrStreamId reliable_output;

uint8_t input_reliable_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_best_effort_stream_buffer[MAX_BUFFER_SIZE];
uint8_t output_reliable_stream_buffer[MAX_BUFFER_SIZE];

#endif // !MICRONODE_H