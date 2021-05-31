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
#include <rmw/allocators.h>

#include "./utils.h"

rmw_guard_condition_t *
rmw_create_guard_condition(
  rmw_context_t * context)
{
  (void)context;

  rmw_guard_condition_t * rmw_guard_condition = (rmw_guard_condition_t *)rmw_allocate(
    sizeof(rmw_guard_condition_t));

  rmw_guard_condition->context = context;
  rmw_guard_condition->implementation_identifier = rmw_get_implementation_identifier();
  rmw_guard_condition->data = (bool *)rmw_allocate(sizeof(bool));

  bool * hasTriggered = (bool *)rmw_guard_condition->data;
  *hasTriggered = false;

  return rmw_guard_condition;
}

rmw_ret_t
rmw_destroy_guard_condition(
  rmw_guard_condition_t * guard_condition)
{
  rmw_free(guard_condition);

  return RMW_RET_OK;
}
