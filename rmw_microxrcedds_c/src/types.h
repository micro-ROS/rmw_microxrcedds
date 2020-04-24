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

#ifndef RMW_MICROXRCEDDS_C__SRC__TYPES_H
#define RMW_MICROXRCEDDS_C__SRC__TYPES_H

#include <stddef.h>

#include <rmw/types.h>
#include <ucdr/microcdr.h>
#include <uxr/client/client.h>

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_typesupport_microxrcedds_c/message_type_support.h"

#include "rosidl_generator_c/service_type_support_struct.h"
#include "rosidl_typesupport_microxrcedds_c/service_type_support.h"

#include "memory.h"
#include <rmw_microxrcedds_c/config.h>

#ifdef MICRO_XRCEDDS_IPV4
  #define MAX_IP_LEN 16
#elif defined(MICRO_XRCEDDS_IPV6)
  #define MAX_IP_LEN 39
#endif
#define MAX_PORT_LEN 5
#define MAX_SERIAL_DEVICE 50

// RMW specific definitions

struct rmw_uxrce_connection_t
{
#if defined(MICRO_XRCEDDS_SERIAL) || defined(MICRO_XRCEDDS_CUSTOM_SERIAL) 
  char serial_device[MAX_SERIAL_DEVICE];
#elif defined(MICRO_XRCEDDS_UDP)
  char agent_address[MAX_IP_LEN];
  char agent_port[MAX_PORT_LEN];
#endif
  uint32_t client_key;
};

struct  rmw_context_impl_t
{
  struct rmw_uxrce_mempool_item_t mem;
  struct rmw_uxrce_connection_t connection_params;

#if defined(MICRO_XRCEDDS_SERIAL) || defined(MICRO_XRCEDDS_CUSTOM_SERIAL)
  uxrSerialTransport transport;
  uxrSerialPlatform serial_platform;
#elif defined(MICRO_XRCEDDS_UDP)
  uxrUDPTransport transport;
  uxrUDPPlatform udp_platform;
#endif
  uxrSession session;

  uxrStreamId reliable_input;
  uxrStreamId reliable_output;
  uxrStreamId best_effort_output;
  uxrStreamId best_effort_input;

  uint8_t input_reliable_stream_buffer[RMW_UXRCE_MAX_BUFFER_SIZE];
  uint8_t output_reliable_stream_buffer[RMW_UXRCE_MAX_BUFFER_SIZE];
  uint8_t output_best_effort_stream_buffer[RMW_UXRCE_MAX_TRANSPORT_MTU];
};

struct  rmw_init_options_impl_t
{
  struct rmw_uxrce_connection_t connection_params;
};

// ROS2 entities definitions

typedef struct rmw_uxrce_topic_t
{
  struct rmw_uxrce_topic_t * next_custom_topic;
  struct rmw_uxrce_topic_t * prev_custom_topic;

  uxrObjectId topic_id;
  const message_type_support_callbacks_t * message_type_support_callbacks;

  bool sync_with_agent;
  int32_t usage_account;
  struct rmw_uxrce_node_t * owner_node;
} rmw_uxrce_topic_t;

typedef struct rmw_uxrce_service_t
{
  struct rmw_uxrce_mempool_item_t mem;
  rmw_service_t * rmw_handle;
  uxrObjectId service_id;
  rmw_gid_t service_gid;
  const service_type_support_callbacks_t * type_support_callbacks;
  uint16_t request_id;

  SampleIdentity sample_id[RMW_UXRCE_MAX_HISTORY];
  uint8_t micro_buffer[RMW_UXRCE_MAX_HISTORY][RMW_UXRCE_MAX_BUFFER_SIZE];
  size_t micro_buffer_lenght[RMW_UXRCE_MAX_HISTORY];

  uint8_t history_write_index;
  uint8_t history_read_index;
  bool micro_buffer_in_use;

  uxrStreamId stream_id;

  uint8_t replay_buffer[RMW_UXRCE_MAX_TRANSPORT_MTU];

  struct rmw_uxrce_node_t * owner_node;
} rmw_uxrce_service_t;

typedef struct rmw_uxrce_client_t
{
  struct rmw_uxrce_mempool_item_t mem;
  rmw_client_t * rmw_handle;
  uxrObjectId client_id;
  rmw_gid_t client_gid;
  const service_type_support_callbacks_t * type_support_callbacks;
  uint16_t request_id;

  int64_t reply_id[RMW_UXRCE_MAX_HISTORY];
  uint8_t micro_buffer[RMW_UXRCE_MAX_HISTORY][RMW_UXRCE_MAX_BUFFER_SIZE];
  size_t micro_buffer_lenght[RMW_UXRCE_MAX_HISTORY];

  uint8_t history_write_index;
  uint8_t history_read_index;
  bool micro_buffer_in_use;

  uxrStreamId stream_id;

  uint8_t request_buffer[RMW_UXRCE_MAX_TRANSPORT_MTU];

  struct rmw_uxrce_node_t * owner_node;
} rmw_uxrce_client_t;

typedef struct rmw_uxrce_subscription_t
{
  struct rmw_uxrce_mempool_item_t mem;
  rmw_subscription_t * rmw_handle;
  uxrObjectId subscriber_id;
  uxrObjectId datareader_id;
  rmw_gid_t subscription_gid;
  const message_type_support_callbacks_t * type_support_callbacks;

  uint8_t micro_buffer[RMW_UXRCE_MAX_HISTORY][RMW_UXRCE_MAX_BUFFER_SIZE];
  size_t micro_buffer_lenght[RMW_UXRCE_MAX_HISTORY];

  uint8_t history_write_index;
  uint8_t history_read_index;
  bool micro_buffer_in_use;

  uint16_t subscription_request;

  struct rmw_uxrce_topic_t * topic;

  struct rmw_uxrce_node_t * owner_node;
  rmw_qos_profile_t qos;
  uxrStreamId stream_id;

} rmw_uxrce_subscription_t;

typedef struct rmw_uxrce_publisher_t
{
  struct rmw_uxrce_mempool_item_t mem;
  rmw_publisher_t  * rmw_handle;
  uxrObjectId publisher_id;
  uxrObjectId datawriter_id;
  rmw_gid_t publisher_gid;

  const message_type_support_callbacks_t * type_support_callbacks;

  struct rmw_uxrce_topic_t * topic;
  
  uxrStreamId stream_id;

  struct rmw_uxrce_node_t * owner_node;
} rmw_uxrce_publisher_t;

typedef struct rmw_uxrce_node_t
{
  struct rmw_uxrce_mempool_item_t mem;
  struct  rmw_context_impl_t * context;

  uxrObjectId participant_id;
  rmw_uxrce_topic_t * custom_topic_sp;
  uint16_t id_gen;
} rmw_uxrce_node_t;

// Static memory pools

extern struct rmw_uxrce_mempool_t session_memory;
extern rmw_context_impl_t custom_sessions[RMW_UXRCE_MAX_SESSIONS];

extern struct rmw_uxrce_mempool_t node_memory;
extern rmw_uxrce_node_t custom_nodes[RMW_UXRCE_MAX_NODES];

extern struct rmw_uxrce_mempool_t publisher_memory;
extern rmw_uxrce_publisher_t custom_publishers[RMW_UXRCE_MAX_PUBLISHERS + RMW_UXRCE_MAX_NODES];

extern struct rmw_uxrce_mempool_t subscription_memory;
extern rmw_uxrce_subscription_t custom_subscriptions[RMW_UXRCE_MAX_SUBSCRIPTIONS];

extern struct rmw_uxrce_mempool_t service_memory;
extern rmw_uxrce_service_t custom_services[RMW_UXRCE_MAX_SERVICES];

extern struct rmw_uxrce_mempool_t client_memory;
extern rmw_uxrce_client_t custom_clients[RMW_UXRCE_MAX_CLIENTS];

// Memory init functions

void rmw_uxrce_init_sessions_memory(struct rmw_uxrce_mempool_t * memory, rmw_context_impl_t * sessions, size_t size);
void rmw_uxrce_init_nodes_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_node_t * nodes, size_t size);
void rmw_uxrce_init_service_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_service_t * services, size_t size);
void rmw_uxrce_init_client_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_client_t * clients, size_t size);
void rmw_uxrce_init_publisher_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_publisher_t * publishers, size_t size);
void rmw_uxrce_init_subscriber_memory(struct rmw_uxrce_mempool_t * memory, rmw_uxrce_subscription_t * subscribers, size_t size);

// Memory management functions

void rmw_uxrce_fini_session_memory(rmw_context_impl_t * session);
void rmw_uxrce_fini_node_memory(rmw_node_t * node);
void rmw_uxrce_fini_publisher_memory(rmw_publisher_t * publisher);
void rmw_uxrce_fini_subscription_memory(rmw_subscription_t * subscriber);
void rmw_uxrce_fini_client_memory(rmw_client_t * client);
void rmw_uxrce_fini_service_memory(rmw_service_t * service);

#endif  // RMW_MICROXRCEDDS_C__SRC__TYPES_H
