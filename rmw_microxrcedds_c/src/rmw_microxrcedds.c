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

#include "rmw_microxrcedds_c/rmw_microxrcedds.h"  // NOLINT

#include <limits.h>

#include <uxr/client/client.h>
#include <rosidl_typesupport_microxrcedds_shared/identifier.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"

#include "./identifiers.h"

#include "./rmw_node.h"
#include "./rmw_publisher.h"
#include "./rmw_subscriber.h"
#include "./types.h"
#include "./utils.h"

rmw_ret_t rmw_publish_serialized_message(
  const rmw_publisher_t * publisher,
  const rmw_serialized_message_t * serialized_message)
{
  EPROS_PRINT_TRACE()
  return RMW_RET_OK;
}


rmw_wait_set_t * rmw_create_wait_set(size_t max_conditions)
{
  EPROS_PRINT_TRACE()

  rmw_wait_set_t * rmw_wait_set = (rmw_wait_set_t *)rmw_allocate(
    sizeof(rmw_wait_set_t));

  return rmw_wait_set;
}

rmw_ret_t rmw_destroy_wait_set(rmw_wait_set_t * wait_set)
{
  EPROS_PRINT_TRACE()

  rmw_free(wait_set);

  return RMW_RET_OK;
}
