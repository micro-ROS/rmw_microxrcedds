#ifndef RMW_NODE_H_
#define RMW_NODE_H_

#include "rmw/rmw.h"

#include <micrortps/client/client.h>

#define EPROS_PRINT_TRACE() printf("func %s, in file %s:%d\n",__PRETTY_FUNCTION__,__FILE__,__LINE__);

// Todo(borja): use static memory allocations with a fixed number of sessions/nodes.
typedef struct MicroNode
{
    mrUDPTransport udp;
    mrSession session;
    mrObjectId participant_id;
    mrObjectId publisher_id;
    mrObjectId datawriter_id;
} MicroNode;

mrStreamId best_input;
mrStreamId reliable_input;
mrStreamId best_output;
mrStreamId reliable_output;
typedef struct Communication Communication;

rmw_node_t* create_node(const char* name, const char* namespace_);

#endif // !RMW_NODE_H_