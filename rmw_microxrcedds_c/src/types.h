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

#ifndef TYPES_H_
#define TYPES_H_

#include <stddef.h>

#include <rmw/types.h>
#include <ucdr/microcdr.h>
#include <uxr/client/client.h>

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_typesupport_microxrcedds_shared/message_type_support.h"

#include "memory.h"
#include <rmw_microxrcedds_c/config.h>

typedef struct custom_topic_t
{
  struct custom_topic_t * next_custom_topic;
  struct custom_topic_t * prev_custom_topic;

  uxrObjectId topic_id;
  const message_type_support_callbacks_t * message_type_support_callbacks;

  bool sync_with_agent;
  int32_t usage_account;
  struct CustomNode * owner_node;
} custom_topic_t;

typedef struct CustomSubscription
{
  struct Item mem;
  uxrObjectId subscriber_id;
  uxrObjectId datareader_id;
  rmw_gid_t subscription_gid;
  const message_type_support_callbacks_t * type_support_callbacks;
  uxrSession * session;  // TODO(Javier) duplicated: owner_node->session

  struct ucdrBuffer micro_buffer;
  bool micro_buffer_in_use;

  bool waiting_for_response;
  uint16_t subscription_request;

  uxrObjectId topic_id;  // TODO(Javier) Pending to be removed
  struct custom_topic_t * topic;

  struct CustomNode * owner_node;
} CustomSubscription;

typedef struct CustomPublisher
{
  struct Item mem;
  uxrObjectId publisher_id;
  uxrObjectId datawriter_id;
  rmw_gid_t publisher_gid;

  const message_type_support_callbacks_t * type_support_callbacks;
  uxrSession * session;  // TODO(Javier) duplicated: owner_node->session

  uxrObjectId topic_id;  // TODO(Javier) Pending to be removed
  struct custom_topic_t * topic;

  struct CustomNode * owner_node;
} CustomPublisher;

typedef struct CustomNode
{
  struct Item mem;
#ifdef MICRO_XRCEDDS_SERIAL
  uxrSerialTransport transport;
  uxrSerialPlatform serial_platform;
#elif defined(MICRO_XRCEDDS_UDP)
  uxrUDPTransport transport;
  uxrUDPPlatform udp_platform;
#endif
  uxrSession session;
  uxrObjectId participant_id;
  struct MemPool publisher_mem;
  struct MemPool subscription_mem;

  CustomPublisher publisher_info[MAX_PUBLISHERS_X_NODE];
  CustomSubscription subscription_info[MAX_SUBSCRIPTIONS_X_NODE];
  custom_topic_t * custom_topic_sp;

  bool on_subscription;

  uxrStreamId reliable_input;
  uxrStreamId reliable_output;

  uint8_t input_reliable_stream_buffer[MAX_BUFFER_SIZE];
  uint8_t output_reliable_stream_buffer[MAX_BUFFER_SIZE];

  uint8_t miscellaneous_temp_buffer[MAX_TRANSPORT_MTU];

  uint16_t id_gen;
} CustomNode;

void init_nodes_memory(struct MemPool * memory, CustomNode nodes[MAX_NODES], size_t size);

#endif  // TYPES_H_
