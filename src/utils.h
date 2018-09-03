#ifndef UTILS_H_
#define UTILS_H_

#include "micronode.h"

#include <rmw/rmw.h>

#define EPROS_PRINT_TRACE() printf("func %s, in file %s:%d\n", __func__, __FILE__, __LINE__);

void rmw_delete(void* rmw_allocated_ptr);
void rmw_node_delete(rmw_node_t* node);
void rmw_publisher_delete(rmw_publisher_t* publisher);

void micronode_clear(MicroNode* node);
void publisherinfo_clear(PublisherInfo* publisher);

#endif // !UTILS_H