// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rmw_microxrcedds_c/config.h>
#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/ret_types.h>
#include <rmw/error_handling.h>

#include "../types.h"

rmw_ret_t rmw_uros_init_options(
  int argc,
  const char * const argv[],
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
    snprintf(rmw_options->impl->transport_params.serial_device, MAX_SERIAL_DEVICE, "%s", argv[1]);
  } else {
    RMW_SET_ERROR_MSG(
      "Wrong number of arguments in rmw options. Needs one argument with the serial device.");
    ret = RMW_RET_INVALID_ARGUMENT;
  }
#elif defined(RMW_UXRCE_TRANSPORT_UDP)
  if (argc >= 3) {
    snprintf(rmw_options->impl->transport_params.agent_address, MAX_IP_LEN, "%s", argv[1]);
    snprintf(rmw_options->impl->transport_params.agent_port, MAX_PORT_LEN, "%s", argv[2]);
  } else {
    RMW_SET_ERROR_MSG("Wrong number of arguments in rmw options. Needs an Agent IP and port.");
    ret = RMW_RET_INVALID_ARGUMENT;
  }
#else
  (void)argc;
  (void)argv;
#endif /* if defined(RMW_UXRCE_TRANSPORT_SERIAL) */
  return ret;
}

rmw_ret_t rmw_uros_options_set_serial_device(
  const char * dev,
  rmw_init_options_t * rmw_options)
{
#if defined(RMW_UXRCE_TRANSPORT_SERIAL)
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (dev != NULL && strlen(dev) <= MAX_SERIAL_DEVICE) {
    snprintf(rmw_options->impl->transport_params.serial_device, MAX_SERIAL_DEVICE, "%s", dev);
  } else {
    RMW_SET_ERROR_MSG("serial port configuration error");
    return RMW_RET_INVALID_ARGUMENT;
  }
  return RMW_RET_OK;
#else
  (void)dev;
  (void)rmw_options;

  RMW_SET_ERROR_MSG("RMW_UXRCE_TRANSPORT_SERIAL not set.");
  return RMW_RET_INVALID_ARGUMENT;
#endif /* if defined(RMW_UXRCE_TRANSPORT_SERIAL) */
}

rmw_ret_t rmw_uros_options_set_udp_address(
  const char * ip,
  const char * port,
  rmw_init_options_t * rmw_options)
{
#ifdef RMW_UXRCE_TRANSPORT_UDP
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (ip != NULL && strlen(ip) <= MAX_IP_LEN) {
    snprintf(rmw_options->impl->transport_params.agent_address, MAX_IP_LEN, "%s", ip);
  } else {
    RMW_SET_ERROR_MSG("default ip configuration error");
    return RMW_RET_INVALID_ARGUMENT;
  }

  if (port != NULL && strlen(port) <= MAX_PORT_LEN) {
    snprintf(rmw_options->impl->transport_params.agent_port, MAX_PORT_LEN, "%s", port);
  } else {
    RMW_SET_ERROR_MSG("default port configuration error");
    return RMW_RET_INVALID_ARGUMENT;
  }

  return RMW_RET_OK;
#else
  (void)ip;
  (void)port;
  (void)rmw_options;

  RMW_SET_ERROR_MSG("RMW_UXRCE_TRANSPORT_UDP not set.");
  return RMW_RET_INVALID_ARGUMENT;
#endif /* ifdef RMW_UXRCE_TRANSPORT_UDP */
}

rmw_ret_t rmw_uros_options_set_client_key(
  uint32_t client_key,
  rmw_init_options_t * rmw_options)
{
  if (NULL == rmw_options) {
    RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
    return RMW_RET_INVALID_ARGUMENT;
  }

  rmw_options->impl->transport_params.client_key = client_key;

  return RMW_RET_OK;
}
