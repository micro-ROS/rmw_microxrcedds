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

#include "rmw_uros/options.h"

#include "types.h"

#include <rmw_microxrcedds_c/config.h>
#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/ret_types.h>
#include <rmw/error_handling.h>

#include <uxr/client/client.h>
#include <uxr/client/util/ping.h>

rmw_ret_t rmw_uros_init_options(
  int argc, const char * const argv[],
  rmw_init_options_t * rmw_options)
{
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }
  rmw_ret_t ret = RMW_RET_OK;
  // TODO(pablogs9): Is the impl allocated at this point?
  // rmw_options->impl = rmw_options->allocator.allocate(
  // sizeof(rmw_init_options_impl_t),
  // rmw_options->allocator.state);
#if defined(RMW_UXRCE_TRANSPORT_SERIAL)
  if (argc >= 2) {
    strcpy(rmw_options->impl->transport_params.serial_device, argv[1]);
  } else {
    RMW_SET_ERROR_MSG(
      "Wrong number of arguments in rmw options. Needs one argument with the serial device.");
    ret = RMW_RET_INVALID_ARGUMENT;
  }

#elif defined(RMW_UXRCE_TRANSPORT_UDP)
  if (argc >= 3) {
    strcpy(rmw_options->impl->transport_params.agent_address, argv[1]);
    strcpy(rmw_options->impl->transport_params.agent_port, argv[2]);
  } else {
    RMW_SET_ERROR_MSG("Wrong number of arguments in rmw options. Needs an Agent IP and port.");
    ret = RMW_RET_INVALID_ARGUMENT;
  }
#else
  (void) argc;
  (void) argv;
#endif
  return ret;
}

rmw_ret_t rmw_uros_options_set_serial_device(const char * dev, rmw_init_options_t * rmw_options)
{
#if defined(RMW_UXRCE_TRANSPORT_SERIAL)
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (dev != NULL && strlen(dev) <= MAX_SERIAL_DEVICE) {
    strcpy(rmw_options->impl->transport_params.serial_device, dev);
  } else {
    RMW_SET_ERROR_MSG("serial port configuration error");
    return RMW_RET_INVALID_ARGUMENT;
  }
  return RMW_RET_OK;
#else
  (void) dev;
  (void) rmw_options;

  RMW_SET_ERROR_MSG("RMW_UXRCE_TRANSPORT_SERIAL not set.");
  return RMW_RET_INVALID_ARGUMENT;
#endif
}

rmw_ret_t rmw_uros_options_set_udp_address(
  const char * ip, const char * port,
  rmw_init_options_t * rmw_options)
{
#ifdef RMW_UXRCE_TRANSPORT_UDP
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (ip != NULL && strlen(ip) <= MAX_IP_LEN) {
    strcpy(rmw_options->impl->transport_params.agent_address, ip);
  } else {
    RMW_SET_ERROR_MSG("default ip configuration error");
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (port != NULL && strlen(port) <= MAX_PORT_LEN) {
    strcpy(rmw_options->impl->transport_params.agent_port, port);
  } else {
    RMW_SET_ERROR_MSG("default port configuration error");
    return RMW_RET_INVALID_ARGUMENT;
  }

  return RMW_RET_OK;
#else
  (void) ip;
  (void) port;
  (void) rmw_options;

  RMW_SET_ERROR_MSG("RMW_UXRCE_TRANSPORT_UDP not set.");
  return RMW_RET_INVALID_ARGUMENT;
#endif
}

#if defined(RMW_UXRCE_TRANSPORT_UDP) && defined(UCLIENT_PROFILE_DISCOVERY)
bool on_agent_found(const TransportLocator* locator, void* args)
{
  rmw_init_options_t * rmw_options = (rmw_init_options_t *) args;
  uxrIpProtocol ip_protocol;
  char ip[MAX_IP_LEN];
  char port_str[MAX_PORT_LEN];
  uint16_t port;

  uxr_locator_to_ip(locator, ip, sizeof(ip), &port, &ip_protocol);
  sprintf(port_str, "%d", port);

  uxrUDPTransport transport;
  if(uxr_init_udp_transport(&transport, ip_protocol, ip, port_str))
  {
    uxrSession session;
    uxr_init_session(&session, &transport.comm, rmw_options->impl->transport_params.client_key);
    if(uxr_create_session_retries(&session, 5))
    {
      sprintf(rmw_options->impl->transport_params.agent_port, "%d", port);
      sprintf(rmw_options->impl->transport_params.agent_address, "%s", ip);
      uxr_delete_session(&session);
      return true;
    }
  }
  return false;
}
#endif

rmw_ret_t rmw_uros_discover_agent(rmw_init_options_t * rmw_options)
{
#if defined(RMW_UXRCE_TRANSPORT_UDP) && defined(UCLIENT_PROFILE_DISCOVERY)
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }

  memset(rmw_options->impl->transport_params.agent_address, 0, MAX_IP_LEN);
  memset(rmw_options->impl->transport_params.agent_port, 0, MAX_PORT_LEN);

  uxr_discovery_agents_default(1, 1000, on_agent_found, (void*) rmw_options);

  return (strlen(rmw_options->impl->transport_params.agent_address) > 0 )? RMW_RET_OK : RMW_RET_TIMEOUT;
#else
  (void) rmw_options;

  RMW_SET_ERROR_MSG("RMW_UXRCE_TRANSPORT_UDP or UCLIENT_PROFILE_DISCOVERY not set.");
  return RMW_RET_INVALID_ARGUMENT;
#endif
}

rmw_ret_t rmw_uros_options_set_client_key(uint32_t client_key, rmw_init_options_t * rmw_options)
{
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }

  rmw_options->impl->transport_params.client_key = client_key;

  return RMW_RET_OK;
}

rmw_ret_t rmw_uros_ping_agent(const int timeout_ms, const uint8_t attempts)
{
  bool success = true;
  if (!session_memory.is_initialized) {
    /* Init transport for performing the ping request */
#ifdef RMW_UXRCE_TRANSPORT_SERIAL
    uxrSerialTransport transport;
    uxrSerialPlatform platform;

    int fd = open(RMW_UXRCE_DEFAULT_SERIAL_DEVICE, O_RDWR | O_NOCTTY);
    if (success = (0 < fd)) {
      struct termios tty_config;
      memset(&tty_config, 0, sizeof(tty_config));
      if (success = (0 == tcgetattr(fd, &tty_config))) {
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

        if (success = (0 == tcsetattr(fd, TCSANOW, &tty_config))) {
          success = uxr_init_serial_transport(
            &transport, &platform, fd, 0, 1);
        }
      }
    }
#elif defined(RMW_UXRCE_TRANSPORT_UDP)
  #ifdef RMW_UXRCE_TRANSPORT_IPV4
    uxrIpProtocol ip_protocol = UXR_IPv4;
  #elif defined(RMW_UXRCE_TRANSPORT_IPV6)
    uxrIpProtocol ip_protocol = UXR_IPv6;
  #endif
    uxrUDPTransport transport;
    uxrUDPPlatform platform;
    success = uxr_init_udp_transport(
        &transport, &platform, ip_protocol,
        RMW_UXRCE_DEFAULT_UDP_IP,
        RMW_UXRCE_DEFAULT_UDP_PORT);
#elif defined(RMW_UXRCE_TRANSPORT_CUSTOM_SERIAL)
    uxrSerialTransport transport;
    uxrSerialPlatform platform;

    int pseudo_fd = atoi(RMW_UXRCE_DEFAULT_SERIAL_DEVICE);
    success = uxr_init_serial_transport(
        &transport, &platform, pseudo_fd, 0, 1);
#endif
    if (success) {
      success = uxr_ping_agent_attempts(&transport.comm, timeout_ms, attempts);
    } else {
      RMW_SET_ERROR_MSG("Error while initializing transport for ping attempt");
      return RMW_RET_ERROR;
    }
#if defined(RMW_UXRCE_TRANSPORT_SERIAL) || defined(RMW_UXRCE_TRANSPORT_CUSTOM_SERIAL)
    uxr_close_serial_transport(&transport);
#elif defined(RMW_UXRCE_TRANSPORT_UDP)
    uxr_close_udp_transport(&transport);
#endif
  } else {
    rmw_uxrce_mempool_item_t * item = session_memory.allocateditems;
    do {
      rmw_context_impl_t * context = (rmw_context_impl_t *)item->data;
      success = uxr_ping_agent_attempts(&context->transport.comm, timeout_ms, attempts);
      item = item->next;
    } while (NULL != item && !success);
  }
  return (success ? RMW_RET_OK : RMW_RET_ERROR);
}

void rmw_uros_set_continous_serialization_callbacks(
  rmw_publisher_t * publisher,
  rmw_uros_continous_serialization_size size_cb, 
  rmw_uros_continous_serialization serialization_cb)
{  
  rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;
  custom_publisher->cs_cb_size = size_cb;
  custom_publisher->cs_cb_serialization = serialization_cb;
}

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM
rmw_uxrce_transport_params_t rmw_uxrce_transport_default_params;

rmw_ret_t rmw_uros_set_custom_transport(
  bool framing,
  void * args,
  open_custom_func open_cb,
  close_custom_func close_cb,
  write_custom_func write_cb,
  read_custom_func read_cb)
{
  if (NULL != open_cb  &&
      NULL != close_cb &&
      NULL != write_cb &&
      NULL != read_cb)
  {
    rmw_uxrce_transport_default_params.framing = framing;
    rmw_uxrce_transport_default_params.args = args;
    rmw_uxrce_transport_default_params.open_cb = open_cb;
    rmw_uxrce_transport_default_params.close_cb = close_cb;
    rmw_uxrce_transport_default_params.write_cb = write_cb;
    rmw_uxrce_transport_default_params.read_cb = read_cb;
  }else{
    RMW_SET_ERROR_MSG("Uninitialised arguments.");
    return RMW_RET_INVALID_ARGUMENT;
  }
  return RMW_RET_OK;
}
#endif //RMW_UXRCE_TRANSPORT_CUSTOM
