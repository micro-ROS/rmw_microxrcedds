#include "utils.h"

#include "rmw/allocators.h"

void custompublisher_clear(CustomPublisher* publisher);
void customsubscription_clear(CustomSubscription* subscription);

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
        customnode_clear((CustomNode*)node->data);
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
        custompublisher_clear((CustomPublisher*)publisher->data);
        publisher->data = NULL;
    }
    rmw_delete(publisher);
}

void custompublisher_clear(CustomPublisher* publisher)
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

void publishers_clear(CustomPublisher publishers[static MAX_PUBLISHERS_X_NODE])
{
    for (size_t i = 0; i < MAX_PUBLISHERS_X_NODE; i++)
    {
        custompublisher_clear(&publishers[i]);
    }
}

void rmw_subscription_delete(rmw_subscription_t* subscriber)
{
    if (subscriber->implementation_identifier)
    {
        subscriber->implementation_identifier = NULL;
    }
    if (subscriber->topic_name)
    {
        rmw_delete((char*)subscriber->topic_name);
    }
    if (subscriber->data)
    {
        customsubscription_clear((CustomSubscription*)subscriber->data);
        subscriber->data = NULL;
    }
    rmw_delete(subscriber);
}

void customsubscription_clear(CustomSubscription* subscription)
{
    if (subscription)
    {
        subscription->in_use = false;
        memset(&subscription->subscriber_id, 0, sizeof(mrObjectId));
        memset(&subscription->datareader_id, 0, sizeof(mrObjectId));
        memset(&subscription->topic_id, 0, sizeof(mrObjectId));
        subscription->subscription_gid.implementation_identifier = NULL;
        memset(&subscription->subscription_gid.data, 0, RMW_GID_STORAGE_SIZE);
        subscription->typesupport_identifier = NULL;
    }
}

void subscriptions_clear(CustomSubscription subscriptions[static MAX_SUBSCRIPTIONS_X_NODE])
{
    for (size_t i = 0; i < MAX_SUBSCRIPTIONS_X_NODE; i++)
    {
        customsubscription_clear(&subscriptions[i]);
    }
}

void customnode_clear(CustomNode* node)
{
    if (node)
    {
        publishers_clear(node->publisher_info);
        free_mem_pool(&node->publisher_mem);
        subscriptions_clear(node->subscription_info);
        free_mem_pool(&node->subscription_mem);
    }
}
