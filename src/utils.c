#include "utils.h"

#include "rmw/allocators.h"

void rmw_free_and_null(void* rmw_allocated_ptr)
{
    rmw_free(rmw_allocated_ptr);
    rmw_allocated_ptr = NULL;
}

void rmw_node_free_and_null(rmw_node_t* node)
{
    rmw_node_free(node);
    node = NULL;
}

void rmw_node_free_and_null_all(rmw_node_t* node)
{
    rmw_free_and_null((char*)node->namespace_);
    rmw_free_and_null((char*)node->name);
    rmw_node_free_and_null(node);
}