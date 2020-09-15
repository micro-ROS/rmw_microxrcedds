// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <time.h>

#include "./types.h"
#include "./rmw_microxrcedds_c/rmw_c_macros.h"
#include "./rmw_node.h"
#include "./identifiers.h"
#include <rmw_microxrcedds_c/config.h>

#include <rmw/rmw.h>
#include <rmw/error_handling.h>
#include <rmw/allocators.h>
#include <uxr/client/util/time.h>

#include "./callbacks.h"

#ifdef MICRO_XRCEDDS_SERIAL
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#endif

#if defined(MICRO_XRCEDDS_SERIAL) || defined(MICRO_XRCEDDS_CUSTOM_SERIAL)
#define CLOSE_TRANSPORT(transport) uxr_close_serial_transport(transport)
#elif defined(MICRO_XRCEDDS_UDP)
#define CLOSE_TRANSPORT(transport) uxr_close_udp_transport(transport)
#else
#define CLOSE_TRANSPORT(transport)
#endif


rmw_ret_t
rmw_init_options_init(rmw_init_options_t * init_options, rcutils_allocator_t allocator)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RMW_RET_INVALID_ARGUMENT);

  if (NULL != init_options->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized init_options");
    return RMW_RET_INVALID_ARGUMENT;
  }

  init_options->instance_id = 0;
  init_options->implementation_identifier = eprosima_microxrcedds_identifier;
  init_options->allocator = allocator;
  init_options->enclave = "/";

  init_options->impl = allocator.allocate(sizeof(rmw_init_options_impl_t), allocator.state);

#if defined(MICRO_XRCEDDS_SERIAL) || defined(MICRO_XRCEDDS_CUSTOM_SERIAL)
  if (strlen(RMW_UXRCE_DEFAULT_SERIAL_DEVICE) <= MAX_SERIAL_DEVICE) {
    strcpy(init_options->impl->connection_params.serial_device, RMW_UXRCE_DEFAULT_SERIAL_DEVICE);
  } else {
    RMW_SET_ERROR_MSG("default serial port configuration overflow");
    return RMW_RET_INVALID_ARGUMENT;
  }
#elif defined(MICRO_XRCEDDS_UDP)
  if (strlen(RMW_UXRCE_DEFAULT_UDP_IP) <= MAX_IP_LEN) {
    strcpy(init_options->impl->connection_params.agent_address, RMW_UXRCE_DEFAULT_UDP_IP);
  } else {
    RMW_SET_ERROR_MSG("default ip configuration overflow");
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (strlen(RMW_UXRCE_DEFAULT_UDP_PORT) <= MAX_PORT_LEN) {
    strcpy(init_options->impl->connection_params.agent_port, RMW_UXRCE_DEFAULT_UDP_PORT);
  } else {
    RMW_SET_ERROR_MSG("default port configuration overflow");
    return RMW_RET_INVALID_ARGUMENT;
  }
#endif

  srand(uxr_nanos());

  do {
    init_options->impl->connection_params.client_key = rand();
  } while (init_options->impl->connection_params.client_key == 0);

  return RMW_RET_OK;
}

rmw_ret_t
rmw_init_options_copy(const rmw_init_options_t * src, rmw_init_options_t * dst)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(src, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(dst, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    src,
    src->implementation_identifier,
    eprosima_microxrcedds_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  if (NULL != dst->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized dst");
    return RMW_RET_INVALID_ARGUMENT;
  }
  memcpy(dst, src, sizeof(rmw_init_options_t));
  dst->impl = rmw_allocate(sizeof(rmw_init_options_impl_t));
  memcpy(dst->impl, src->impl, sizeof(rmw_init_options_impl_t));

  return RMW_RET_OK;
}

rmw_ret_t
rmw_init_options_fini(rmw_init_options_t * init_options)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&(init_options->allocator), return RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    init_options,
    init_options->implementation_identifier,
    eprosima_microxrcedds_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  rmw_free(init_options->impl);

  *init_options = rmw_get_zero_initialized_init_options();
  return RMW_RET_OK;
}

rmw_ret_t
rmw_init(const rmw_init_options_t * options, rmw_context_t * context)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(options->impl, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    options,
    options->implementation_identifier,
    eprosima_microxrcedds_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  context->instance_id = options->instance_id;
  context->implementation_identifier = eprosima_microxrcedds_identifier;

  rmw_uxrce_init_sessions_memory(&session_memory, custom_sessions, RMW_UXRCE_MAX_SESSIONS);

  struct rmw_uxrce_mempool_item_t * memory_node = get_memory(&session_memory);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available session memory node");
    return RMW_RET_ERROR;
  }

  rmw_context_impl_t * context_impl = (rmw_context_impl_t *)memory_node->data;

  #if defined(MICRO_XRCEDDS_SERIAL) || defined(MICRO_XRCEDDS_CUSTOM_SERIAL)
  strcpy(
    context_impl->connection_params.serial_device,
    options->impl->connection_params.serial_device);
  #elif defined(MICRO_XRCEDDS_UDP)
  strcpy(
    context_impl->connection_params.agent_address,
    options->impl->connection_params.agent_address);
  strcpy(context_impl->connection_params.agent_port, options->impl->connection_params.agent_port);
  #endif

  context_impl->connection_params.client_key = options->impl->connection_params.client_key;

  context_impl->id_participant = 0;
  context_impl->id_topic = 0;
  context_impl->id_publisher = 0;
  context_impl->id_datawriter = 0;
  context_impl->id_subscriber = 0;
  context_impl->id_datareader = 0;
  context_impl->id_requester = 0;
  context_impl->id_replier = 0;

  context->impl = context_impl;

  rmw_uxrce_init_nodes_memory(&node_memory, custom_nodes, RMW_UXRCE_MAX_NODES);
  rmw_uxrce_init_subscriber_memory(
    &subscription_memory, custom_subscriptions,
    RMW_UXRCE_MAX_SUBSCRIPTIONS);
  rmw_uxrce_init_publisher_memory(&publisher_memory, custom_publishers, RMW_UXRCE_MAX_PUBLISHERS);
  rmw_uxrce_init_service_memory(&service_memory, custom_services, RMW_UXRCE_MAX_SERVICES);
  rmw_uxrce_init_client_memory(&client_memory, custom_clients, RMW_UXRCE_MAX_CLIENTS);
  rmw_uxrce_init_topics_memory(&topics_memory, custom_topics, RMW_UXRCE_MAX_TOPICS_INTERNAL);

  // Micro-XRCE-DDS Client initialization

#ifdef MICRO_XRCEDDS_SERIAL
  int fd = open(context->impl->connection_params.serial_device, O_RDWR | O_NOCTTY);
  if (0 < fd) {
    struct termios tty_config;
    memset(&tty_config, 0, sizeof(tty_config));
    if (0 == tcgetattr(fd, &tty_config)) {
      /* Setting CONTROL OPTIONS. */
      tty_config.c_cflag |= CREAD;          // Enable read.
      tty_config.c_cflag |= CLOCAL;         // Set local mode.
      tty_config.c_cflag &= ~PARENB;        // Disable parity.
      tty_config.c_cflag &= ~CSTOPB;        // Set one stop bit.
      tty_config.c_cflag &= ~CSIZE;         // Mask the character size bits.
      tty_config.c_cflag |= CS8;            // Set 8 data bits.
      tty_config.c_cflag &= ~CRTSCTS;       // Disable hardware flow control.

      /* Setting LOCAL OPTIONS. */
      tty_config.c_lflag &= ~ICANON;        // Set non-canonical input.
      tty_config.c_lflag &= ~ECHO;          // Disable echoing of input characters.
      tty_config.c_lflag &= ~ECHOE;         // Disable echoing the erase character.
      tty_config.c_lflag &= ~ISIG;          // Disable SIGINTR, SIGSUSP, SIGDSUSP
                                            // and SIGQUIT signals.

      /* Setting INPUT OPTIONS. */
      tty_config.c_iflag &= ~IXON;          // Disable output software flow control.
      tty_config.c_iflag &= ~IXOFF;         // Disable input software flow control.
      tty_config.c_iflag &= ~INPCK;         // Disable parity check.
      tty_config.c_iflag &= ~ISTRIP;        // Disable strip parity bits.
      tty_config.c_iflag &= ~IGNBRK;        // No ignore break condition.
      tty_config.c_iflag &= ~IGNCR;         // No ignore carrier return.
      tty_config.c_iflag &= ~INLCR;         // No map NL to CR.
      tty_config.c_iflag &= ~ICRNL;         // No map CR to NL.

      /* Setting OUTPUT OPTIONS. */
      tty_config.c_oflag &= ~OPOST;         // Set raw output.

      /* Setting OUTPUT CHARACTERS. */
      tty_config.c_cc[VMIN] = 34;
      tty_config.c_cc[VTIME] = 10;

      /* Setting BAUD RATE. */
      cfsetispeed(&tty_config, B115200);
      cfsetospeed(&tty_config, B115200);

      if (0 == tcsetattr(fd, TCSANOW, &tty_config)) {
        if (!uxr_init_serial_transport(
            &context_impl->transport,
            &context_impl->serial_platform, fd, 0, 1))
        {
          RMW_SET_ERROR_MSG("Can not create an serial connection");
          return RMW_RET_ERROR;
        }
      }
    }
  }
  printf("Serial mode => dev: %s\n", context_impl->connection_params.serial_device);

#elif defined(MICRO_XRCEDDS_UDP)
  // TODO(Borja) Think how we are going to select transport to use
  #ifdef MICRO_XRCEDDS_IPV4
  uxrIpProtocol ip_protocol = UXR_IPv4;
  #elif defined(MICRO_XRCEDDS_IPV6)
  uxrIpProtocol ip_protocol = UXR_IPv6;
  #endif

  if (!uxr_init_udp_transport(
      &context_impl->transport, &context_impl->udp_platform, ip_protocol,
      context_impl->connection_params.agent_address, context_impl->connection_params.agent_port))
  {
    RMW_SET_ERROR_MSG("Can not create an udp connection");
    return RMW_RET_ERROR;
  }
  printf(
    "UDP mode => ip: %s - port: %s\n", context_impl->connection_params.agent_address,
    context_impl->connection_params.agent_port);
#elif defined(MICRO_XRCEDDS_CUSTOM_SERIAL)
  int pseudo_fd = 0;
  if (strlen(options->impl->connection_params.serial_device) > 0) {
    pseudo_fd = atoi(options->impl->connection_params.serial_device);
  }

  if (!uxr_init_serial_transport(
      &context_impl->transport, &context_impl->serial_platform,
      pseudo_fd, 0, 1))
  {
    RMW_SET_ERROR_MSG("Can not create an custom serial connection");
    return RMW_RET_ERROR;
  }
#endif

  uxr_init_session(
    &context_impl->session, &context_impl->transport.comm,
    context_impl->connection_params.client_key);

  uxr_set_topic_callback(&context_impl->session, on_topic, NULL);
  uxr_set_status_callback(&context_impl->session, on_status, NULL);
  uxr_set_request_callback(&context_impl->session, on_request, NULL);
  uxr_set_reply_callback(&context_impl->session, on_reply, NULL);

  context_impl->reliable_input = uxr_create_input_reliable_stream(
    &context_impl->session, context_impl->input_reliable_stream_buffer,
    context_impl->transport.comm.mtu * RMW_UXRCE_STREAM_HISTORY, RMW_UXRCE_STREAM_HISTORY);
  context_impl->reliable_output =
    uxr_create_output_reliable_stream(
    &context_impl->session, context_impl->output_reliable_stream_buffer,
    context_impl->transport.comm.mtu * RMW_UXRCE_STREAM_HISTORY, RMW_UXRCE_STREAM_HISTORY);

  context_impl->best_effort_input = uxr_create_input_best_effort_stream(&context_impl->session);
  context_impl->best_effort_output = uxr_create_output_best_effort_stream(
    &context_impl->session,
    context_impl->output_best_effort_stream_buffer, context_impl->transport.comm.mtu);


  if (!uxr_create_session(&context_impl->session)) {
    CLOSE_TRANSPORT(&context_impl->transport);
    RMW_SET_ERROR_MSG("failed to create node session on Micro ROS Agent.");
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

rmw_ret_t
rmw_shutdown(rmw_context_t * context)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    context,
    context->implementation_identifier,
    eprosima_microxrcedds_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  
  rmw_ret_t ret = rmw_context_fini(context);

  if (RMW_RET_OK == ret) {
    *context = rmw_get_zero_initialized_context();
  }

  return ret;
}

rmw_ret_t
rmw_context_fini(rmw_context_t * context)
{
  // TODO(pablogs9): Should we manage not closed XRCE sessions?
  rmw_ret_t ret = RMW_RET_OK;

  struct rmw_uxrce_mempool_item_t * item = node_memory.allocateditems;

  while (item != NULL) {
    rmw_uxrce_node_t * custom_node = (rmw_uxrce_node_t *)item->data;
    item = item->next;
    if (custom_node->context == context->impl) {
      ret = rmw_destroy_node(custom_node->rmw_handle);
    }
  }

  uxr_delete_session(&context->impl->session);
  rmw_uxrce_fini_session_memory(context->impl);

  CLOSE_TRANSPORT(&context->impl->transport);

  context->impl = NULL;

  return ret;
}
