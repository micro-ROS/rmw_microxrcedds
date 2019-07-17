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

#include "utils.h"

#include <rmw/rmw.h>
#include <rmw/allocators.h>

rmw_client_t * rmw_create_client(
  const rmw_node_t * node, const rosidl_service_type_support_t * type_support,
  const char * service_name, const rmw_qos_profile_t * qos_policies)
{
  EPROS_PRINT_TRACE()
  rmw_client_t * rmw_client = (rmw_client_t *)rmw_allocate(
    sizeof(rmw_client_t));
  return rmw_client;
}

rmw_ret_t rmw_destroy_client(rmw_node_t * node, rmw_client_t * client)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}
