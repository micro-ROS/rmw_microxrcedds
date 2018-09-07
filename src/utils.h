#ifndef RMW_MICRORTPS_UTILS_H_
#define RMW_MICRORTPS_UTILS_H_

#include "types.h"

#include <rmw/rmw.h>

#define EPROS_PRINT_TRACE() ;//printf("func %s, in file %s:%d\n", __func__, __FILE__, __LINE__);

void rmw_delete(void* rmw_allocated_ptr);
void rmw_node_delete(rmw_node_t* node);
void rmw_publisher_delete(rmw_publisher_t* publisher);
void rmw_subscription_delete(rmw_subscription_t* subscriber);

void customnode_clear(CustomNode* node);

#endif // !UTILS_H