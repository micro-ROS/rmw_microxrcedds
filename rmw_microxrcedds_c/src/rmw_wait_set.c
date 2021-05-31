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
#include <rmw/allocators.h>

#include "./utils.h"

rmw_wait_set_t *
rmw_create_wait_set(
  rmw_context_t * context,
  size_t max_conditions)
{
  (void)context;
  (void)max_conditions;

  rmw_wait_set_t * rmw_wait_set = (rmw_wait_set_t *)rmw_allocate(
    sizeof(rmw_wait_set_t));

  return rmw_wait_set;
}

rmw_ret_t
rmw_destroy_wait_set(
  rmw_wait_set_t * wait_set)
{
  rmw_free(wait_set);

  return RMW_RET_OK;
}
