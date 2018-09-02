#ifndef UTILS_H_
#define UTILS_H_

#include <rmw/rmw.h>

#define EPROS_PRINT_TRACE() printf("func %s, in file %s:%d\n", __func__, __FILE__, __LINE__);

void rmw_free_and_null(void* rmw_allocated_ptr);
void rmw_node_free_and_null(rmw_node_t* node);
void rmw_node_free_and_null_all(rmw_node_t* node);

#endif // !UTILS_H