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

#include <rmw/rmw.h>
#include <rmw/error_handling.h>

#include "./types.h"

rmw_ret_t
rmw_get_gid_for_publisher(
  const rmw_publisher_t * publisher,
  rmw_gid_t * gid)
{
  // Check
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(gid, RMW_RET_INVALID_ARGUMENT);
  if (publisher->implementation_identifier != rmw_get_implementation_identifier()) {
    RMW_SET_ERROR_MSG("publisher handle not from this implementation");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  // Do
  rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;

  if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE) {
    RMW_SET_ERROR_MSG("Not enough memory for impl ids");
    return RMW_RET_ERROR;
  }

  memset(gid->data, 0, RMW_GID_STORAGE_SIZE);
  memcpy(
    gid->data,
    &custom_publisher->publisher_id,
    sizeof(uxrObjectId));


  return RMW_RET_OK;
}
