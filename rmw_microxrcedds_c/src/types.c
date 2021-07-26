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

#include <types.h>

#ifdef HAVE_C_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#endif /* ifdef HAVE_C_TYPESUPPORT */
#ifdef HAVE_CPP_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_cpp/identifier.h>
#endif /* ifdef HAVE_CPP_TYPESUPPORT */

#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include <uxr/client/profile/multithread/multithread.h>

#include "./utils.h"
#include "./memory.h"
#include "./rmw_microxrcedds_topic.h"

// Static memory pools

char rmw_uxrce_entity_naming_buffer[RMW_UXRCE_ENTITY_NAMING_BUFFER_LENGTH];

rmw_uxrce_mempool_t session_memory;
rmw_context_impl_t custom_sessions[RMW_UXRCE_MAX_SESSIONS];

rmw_uxrce_mempool_t node_memory;
rmw_uxrce_node_t custom_nodes[RMW_UXRCE_MAX_NODES];

rmw_uxrce_mempool_t publisher_memory;
rmw_uxrce_publisher_t custom_publishers[RMW_UXRCE_MAX_PUBLISHERS];

rmw_uxrce_mempool_t subscription_memory;
rmw_uxrce_subscription_t custom_subscriptions[RMW_UXRCE_MAX_SUBSCRIPTIONS];

rmw_uxrce_mempool_t service_memory;
rmw_uxrce_service_t custom_services[RMW_UXRCE_MAX_SERVICES];

rmw_uxrce_mempool_t client_memory;
rmw_uxrce_client_t custom_clients[RMW_UXRCE_MAX_CLIENTS];

rmw_uxrce_mempool_t topics_memory;
rmw_uxrce_topic_t custom_topics[RMW_UXRCE_MAX_TOPICS_INTERNAL];

rmw_uxrce_mempool_t static_buffer_memory;
rmw_uxrce_static_input_buffer_t custom_static_buffers[RMW_UXRCE_MAX_HISTORY];

// Memory init functions

#define RMW_INIT_MEMORY(X) \
  void rmw_uxrce_init_ ## X ## _memory( \
    rmw_uxrce_mempool_t * memory, \
    rmw_uxrce_ ## X ## _t * array, \
    size_t size) \
  { \
    if (size > 0 && !memory->is_initialized) { \
      UXR_INIT_LOCK(&memory->mutex); \
      memory->is_initialized = true; \
      memory->element_size = sizeof(*array); \
      memory->allocateditems = NULL; \
      memory->freeitems = NULL; \
      memory->is_dynamic_allowed = true; \
 \
      for (size_t i = 0; i < size; i++) { \
        put_memory(memory, &array[i].mem); \
        array[i].mem.data = (void *)&array[i]; \
        array[i].mem.is_dynamic_memory = false; \
      } \
    } \
  }


RMW_INIT_MEMORY(service)
RMW_INIT_MEMORY(client)
RMW_INIT_MEMORY(publisher)
RMW_INIT_MEMORY(subscription)
RMW_INIT_MEMORY(node)
RMW_INIT_MEMORY(session)
RMW_INIT_MEMORY(topic)
RMW_INIT_MEMORY(static_input_buffer)

// Memory management functions

void rmw_uxrce_fini_session_memory(
  rmw_context_impl_t * session)
{
  put_memory(&session_memory, &session->mem);
}

void rmw_uxrce_fini_node_memory(
  rmw_node_t * node)
{
  if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier)) {
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
    custom_node->rmw_handle = NULL;
    custom_node->context = NULL;

    put_memory(&node_memory, &custom_node->mem);

    node->data = NULL;
  }

  rmw_node_free(node);
  node = NULL;
}

void rmw_uxrce_fini_publisher_memory(
  rmw_publisher_t * publisher)
{
  if (!is_uxrce_rmw_identifier_valid(publisher->implementation_identifier)) {
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

    custom_publisher->rmw_handle = NULL;

    put_memory(&publisher_memory, &custom_publisher->mem);
    publisher->data = NULL;
  }

  rmw_free(publisher);
}

void rmw_uxrce_fini_subscription_memory(
  rmw_subscription_t * subscriber)
{
  if (!is_uxrce_rmw_identifier_valid(subscriber->implementation_identifier)) {
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

    custom_subscription->rmw_handle = NULL;

    put_memory(&subscription_memory, &custom_subscription->mem);
    subscriber->data = NULL;
  }
  rmw_free(subscriber);
}

void rmw_uxrce_fini_service_memory(
  rmw_service_t * service)
{
  if (!is_uxrce_rmw_identifier_valid(service->implementation_identifier)) {
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
    custom_service->rmw_handle = NULL;

    put_memory(&service_memory, &custom_service->mem);
    service->data = NULL;
  }
  rmw_free(service);
}

void rmw_uxrce_fini_client_memory(
  rmw_client_t * client)
{
  if (!is_uxrce_rmw_identifier_valid(client->implementation_identifier)) {
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
    custom_client->rmw_handle = NULL;

    put_memory(&client_memory, &custom_client->mem);
    client->data = NULL;
  }
  rmw_free(client);
}

void rmw_uxrce_fini_topic_memory(
  rmw_uxrce_topic_t * topic)
{
  put_memory(&topics_memory, &topic->mem);
  topic->owner_node = NULL;
}

rmw_uxrce_mempool_item_t * rmw_uxrce_find_static_input_buffer_by_owner(
  void * owner)
{
  rmw_uxrce_mempool_item_t * static_buffer_item = static_buffer_memory.allocateditems;
  while (static_buffer_item != NULL) {
    rmw_uxrce_static_input_buffer_t * data =
      (rmw_uxrce_static_input_buffer_t *)static_buffer_item->data;
    if (data->owner == owner) {
      return static_buffer_item;
    }
    static_buffer_item = static_buffer_item->next;
  }
  return NULL;
}
