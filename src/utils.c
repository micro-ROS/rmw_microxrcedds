#include "utils.h"

#include "rmw/allocators.h"

void rmw_delete(void* rmw_allocated_ptr)
{
    rmw_free(rmw_allocated_ptr);
    rmw_allocated_ptr = NULL;
}

void rmw_node_delete(rmw_node_t* node)
{
    if (node->namespace_)
    {
        rmw_delete((char*)node->namespace_);
    }
    if (node->name)
    {
        rmw_delete((char*)node->name);
    }
    if (node->implementation_identifier)
    {
        node->implementation_identifier = NULL;
    }
    if (node->data)
    {
        micronode_clear((MicroNode*)node->data);
        node->data = NULL;
    }

    rmw_node_free(node);
    node = NULL;
}

void rmw_publisher_delete(rmw_publisher_t* publisher)
{
    if (publisher->implementation_identifier)
    {
        publisher->implementation_identifier = NULL;
    }
    if (publisher->topic_name)
    {
        rmw_delete((char*)publisher->topic_name);
    }
    if (publisher->data)
    {
        publisherinfo_clear((PublisherInfo*)publisher->data);
        publisher->data = NULL;
    }
    rmw_delete(publisher);
}

void publishers_clear(PublisherInfo publishers[static MAX_PUBLISHERS])
{
    for (size_t i = 0; i < MAX_PUBLISHERS; i++)
    {
        publisherinfo_clear(&publishers[i]);
    }
}

void publisherinfo_clear(PublisherInfo* publisher)
{
    if (publisher)
    {
        publisher->in_use = false;
        memset(&publisher->publisher_id, 0, sizeof(mrObjectId));
        memset(&publisher->datawriter_id, 0, sizeof(mrObjectId));
        memset(&publisher->topic_id, 0, sizeof(mrObjectId));
        publisher->publisher_gid.implementation_identifier = NULL;
        memset(&publisher->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
        publisher->typesupport_identifier = NULL;
    }
}

void micronode_clear(MicroNode* node)
{
    if (node)
    {
        memset(&node->participant_id, 0, sizeof(mrObjectId));
        publishers_clear(node->publisher_info);
        node->num_publishers = 0;
    }
}
