// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "utils.h"

#include "./types.h"  // NOLINT
#include "./memory.h"
#include "./rmw_microxrcedds_topic.h"

#ifdef HAVE_C_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#endif
#ifdef HAVE_CPP_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_cpp/identifier.h>
#endif

#include "rmw/allocators.h"
#include <rmw/error_handling.h>

// Static memory pools

struct rmw_uxrce_mempool_t node_memory;
rmw_uxrce_node_t custom_nodes[RMW_UXRCE_MAX_NODES];

struct rmw_uxrce_mempool_t publisher_memory;
rmw_uxrce_publisher_t custom_publishers[RMW_UXRCE_MAX_PUBLISHERS + RMW_UXRCE_MAX_NODES];

struct rmw_uxrce_mempool_t subscription_memory;
rmw_uxrce_subscription_t custom_subscriptions[RMW_UXRCE_MAX_SUBSCRIPTIONS];

struct rmw_uxrce_mempool_t service_memory;
rmw_uxrce_service_t custom_services[RMW_UXRCE_MAX_SERVICES];

struct rmw_uxrce_mempool_t client_memory;
rmw_uxrce_client_t custom_clients[RMW_UXRCE_MAX_CLIENTS];

// Memory init functions

void rmw_uxrce_init_service_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_service_t * services, size_t size)
{
  if (size > 0) {
    link_prev(NULL, &services[0].mem, NULL);
    size > 1 ? link_next(&services[0].mem, &services[1].mem, &services[0]) : link_next(
      &services[0].mem, NULL, &services[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&services[i - 1].mem, &services[i].mem, &services[i]);
    }
    link_next(&services[size - 1].mem, NULL, &services[size - 1]);
    set_mem_pool(memory, &services[0].mem);
  }
}

void rmw_uxrce_init_client_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_client_t * clients, size_t size)
{
  if (size > 0) {
    link_prev(NULL, &clients[0].mem, NULL);
    size > 1 ? link_next(&clients[0].mem, &clients[1].mem, &clients[0]) : link_next(
      &clients[0].mem, NULL, &clients[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&clients[i - 1].mem, &clients[i].mem, &clients[i]);
    }
    link_next(&clients[size - 1].mem, NULL, &clients[size - 1]);
    set_mem_pool(memory, &clients[0].mem);
  }
}

void rmw_uxrce_init_publisher_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_publisher_t * publishers, size_t size)
{
  if (size > 0) {
    link_prev(NULL, &publishers[0].mem, NULL);
    size > 1 ? link_next(&publishers[0].mem, &publishers[1].mem, &publishers[0]) : link_next(
      &publishers[0].mem, NULL, &publishers[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&publishers[i - 1].mem, &publishers[i].mem, &publishers[i]);
    }
    link_next(&publishers[size - 1].mem, NULL, &publishers[size - 1]);
    set_mem_pool(memory, &publishers[0].mem);
  }
}

void rmw_uxrce_init_subscriber_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_subscription_t * subscribers, size_t size)
{
  if (size > 0) {
    link_prev(NULL, &subscribers[0].mem, NULL);
    size > 1 ? link_next(&subscribers[0].mem, &subscribers[1].mem, &subscribers[0]) : link_next(
      &subscribers[0].mem, NULL, &subscribers[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&subscribers[i - 1].mem, &subscribers[i].mem, &subscribers[i]);
    }
    link_next(&subscribers[size - 1].mem, NULL, &subscribers[size - 1]);
    set_mem_pool(memory, &subscribers[0].mem);
  }
}

void rmw_uxrce_init_nodes_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_node_t * nodes, size_t size)
{
  if (size > 0) {
    link_prev(NULL, &nodes[0].mem, NULL);
    size > 1 ? link_next(&nodes[0].mem, &nodes[1].mem, &nodes[0]) : link_next(&nodes[0].mem, NULL,
      &nodes[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&nodes[i - 1].mem, &nodes[i].mem, &nodes[i]);
    }
    link_next(&nodes[size - 1].mem, NULL, &nodes[size - 1]);
    set_mem_pool(memory, &nodes[0].mem);
  }
}

// Memory management functions

  void * data;

void rmw_uxrce_fini_node_memory(rmw_node_t * node)
{ 
  if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return;
  }
  if (node->namespace_) {
    rmw_free((char *)node->namespace_);
  }
  if (node->name) {
    rmw_free((char *)node->name);
  }
  if (node->implementation_identifier) {
    node->implementation_identifier = NULL;
  }
  if (node->data) {
    rmw_uxrce_node_t * custom_node = (rmw_uxrce_node_t *)node->data;
    put_memory(&node_memory, &custom_node->mem);

    memset(custom_node, 0, sizeof(rmw_uxrce_node_t));
    node->data = NULL;
  }

  rmw_node_free(node);
  node = NULL;
}

void rmw_uxrce_fini_publisher_memory(rmw_publisher_t * publisher)
{
  if (strcmp(publisher->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return;
  }
  if (publisher->implementation_identifier) {
    publisher->implementation_identifier = NULL;
  }
  if (publisher->topic_name) {
    rmw_free((char *)publisher->topic_name);
  }
  if (publisher->data) {
    rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;

    if (custom_publisher->topic != NULL) {
      destroy_topic(custom_publisher->topic);
    }

    put_memory(&publisher_memory, &custom_publisher->mem);

    memset(custom_publisher, 0, sizeof(rmw_uxrce_publisher_t));
    publisher->data = NULL;
  }

  rmw_free(publisher);
}

void rmw_uxrce_fini_subscription_memory(rmw_subscription_t * subscriber)
{
  if (strcmp(subscriber->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return;
  }
  if (subscriber->implementation_identifier) {
    subscriber->implementation_identifier = NULL;
  }
  if (subscriber->topic_name) {
    rmw_free((char *)subscriber->topic_name);
  }
  if (subscriber->data) {
    rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)subscriber->data;

    if (custom_subscription->topic != NULL) {
      destroy_topic(custom_subscription->topic);
    }

    put_memory(&subscription_memory, &custom_subscription->mem);

    memset(custom_subscription, 0, sizeof(rmw_uxrce_subscription_t));
    subscriber->data = NULL;
  }
  rmw_free(subscriber);
}

void rmw_uxrce_fini_service_memory(rmw_service_t * service)
{
  if (strcmp(service->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return;
  }
  if (service->implementation_identifier) {
    service->implementation_identifier = NULL;
  }
  if (service->service_name) {
    rmw_free((char *)service->service_name);
  }
  if (service->data) {
    rmw_uxrce_service_t * custom_service = (rmw_uxrce_service_t *)service->data;

    put_memory(&service_memory,&custom_service->mem);

    memset(custom_service, 0, sizeof(rmw_uxrce_service_t));
    service->data = NULL;
  }
  rmw_free(service);
}

void rmw_uxrce_fini_client_memory(rmw_client_t * client)
{
  if (strcmp(client->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return;
  }
  if (client->implementation_identifier) {
    client->implementation_identifier = NULL;
  }
  if (client->service_name) {
    rmw_free((char *)client->service_name);
  }
  if (client->data) {
    rmw_uxrce_client_t * custom_client = (rmw_uxrce_client_t *)client->data;

    put_memory(&client_memory, &custom_client->mem);

    memset(custom_client, 0, sizeof(rmw_uxrce_client_t));
    client->data = NULL;
  }
  rmw_free(client);
}
