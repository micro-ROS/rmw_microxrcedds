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

#include "./memory.h"
#include "./config.h"


typedef struct CustomSubscription
{
  uxrObjectId subscriber_id;
  uxrObjectId datareader_id;
  uxrObjectId topic_id;
  rmw_gid_t subscription_gid;
  const message_type_support_callbacks_t * type_support_callbacks;
  struct CustomNode * owner_node;
  uxrSession * session;

  struct ucdrBuffer micro_buffer;

  bool waiting_for_response;
  uint16_t subscription_request;

  struct Item mem;
} CustomSubscription;

typedef struct CustomPublisher
{
  uxrObjectId publisher_id;
  uxrObjectId datawriter_id;
  uxrObjectId topic_id;
  rmw_gid_t publisher_gid;
  const message_type_support_callbacks_t * type_support_callbacks;
  uxrSession * session;
  struct Item mem;

  struct CustomNode * owner_node;
} CustomPublisher;

typedef struct CustomNode
{
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
  struct Item mem;

  CustomPublisher publisher_info[MAX_PUBLISHERS_X_NODE];
  CustomSubscription subscription_info[MAX_SUBSCRIPTIONS_X_NODE];

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
