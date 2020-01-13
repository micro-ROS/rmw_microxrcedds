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

  init_options->impl = allocator.allocate(sizeof(rmw_init_options_impl_t), allocator.state);

#ifdef MICRO_XRCEDDS_SERIAL
  if(strlen(RMW_DEFAULT_SERIAL_DEVICE) <= strlen(init_options->impl->serial_device)){
    strcpy(init_options->impl->serial_device, RMW_DEFAULT_SERIAL_DEVICE);
  }else{
    RMW_SET_ERROR_MSG("default serial port configuration overflow");
    return RMW_RET_INVALID_ARGUMENT;
  }
#elif defined(MICRO_XRCEDDS_UDP)
  if(strlen(RMW_DEFAULT_UDP_IP) <= strlen(init_options->impl->agent_address)){
    strcpy(init_options->impl->agent_address, RMW_DEFAULT_UDP_IP);
  }else{
    RMW_SET_ERROR_MSG("default ip configuration overflow");
    return RMW_RET_INVALID_ARGUMENT;
  }
  
  if(strlen(RMW_DEFAULT_UDP_PORT) <= strlen(init_options->impl->agent_port)){
    strcpy(init_options->impl->agent_port, RMW_DEFAULT_UDP_PORT);
  }else{
    RMW_SET_ERROR_MSG("default port configuration overflow");
    return RMW_RET_INVALID_ARGUMENT;
  }
#endif

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
  *dst = *src;
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

  rmw_context_impl_t * context_impl = (rmw_context_impl_t *)rmw_allocate(sizeof(rmw_context_impl_t));
  #ifdef MICRO_XRCEDDS_SERIAL
    strcpy(context_impl->serial_device, options->impl->serial_device);
  #elif defined(MICRO_XRCEDDS_UDP)
    strcpy(context_impl->agent_address, options->impl->agent_address);
    strcpy(context_impl->agent_port, options->impl->agent_port);
  #endif
  context->impl = context_impl;

  // Intialize random number generator
  time_t t;
  srand((unsigned)time(&t));

  init_rmw_node();

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
  // context impl is explicitly supposed to be nullptr for now, see rmw_init's code
  // RCUTILS_CHECK_ARGUMENT_FOR_NULL(context->impl, RMW_RET_INVALID_ARGUMENT);
  *context = rmw_get_zero_initialized_context();
  return RMW_RET_OK;
}

rmw_ret_t
rmw_context_fini(rmw_context_t * context)
{
  (void) context;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}
