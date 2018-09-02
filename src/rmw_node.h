#ifndef RMW_NODE_H_
#define RMW_NODE_H_

#include "rmw/rmw.h"

rmw_node_t* create_node(const char* name, const char* namespace_, size_t domain_id);
rmw_ret_t destroy_node(rmw_node_t* node);

#endif // !RMW_NODE_H_