#ifndef RMW_NODE_H_
#define RMW_NODE_H_

#include "types.h"
#include <rmw/types.h>

rmw_node_t* create_node(const char* name, const char* namespace_, size_t domain_id);
void init_rmw_node();

#endif // !RMW_NODE_H_