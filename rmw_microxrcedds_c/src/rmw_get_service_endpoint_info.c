// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rmw/rmw.h>

#include <rmw/get_service_endpoint_info.h>
#include <rmw/types.h>

#include "./rmw_microros_internal/error_handling_internal.h"

rmw_ret_t
rmw_get_clients_info_by_service(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * service_name,
  bool no_mangle,
  rmw_service_endpoint_info_array_t * clients_info)
{
  (void)node;
  (void)allocator;
  (void)service_name;
  (void)no_mangle;
  (void)clients_info;
  RMW_UROS_TRACE_MESSAGE("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_get_servers_info_by_service(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * service_name,
  bool no_mangle,
  rmw_service_endpoint_info_array_t * servers_info)
{
  (void)node;
  (void)allocator;
  (void)service_name;
  (void)no_mangle;
  (void)servers_info;
  RMW_UROS_TRACE_MESSAGE("function not implemented");
  return RMW_RET_UNSUPPORTED;
}
