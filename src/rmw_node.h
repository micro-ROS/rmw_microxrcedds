#ifndef RMW_NODE_H_
#define RMW_NODE_H_

#include "rmw/rmw.h"

typedef struct Communication Communication;

rmw_node_t* create_node(const char* name, const char* namespace_);

#endif // !RMW_NODE_H_