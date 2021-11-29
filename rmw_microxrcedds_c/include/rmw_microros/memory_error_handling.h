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

/**
 * @file
 */

#ifndef RMW_MICROROS__MEMORY_ERROR_HANDLING_H_
#define RMW_MICROROS__MEMORY_ERROR_HANDLING_H_

#include <ucdr/microcdr.h>

#if defined(__cplusplus)
extern "C"
{
#endif  // if defined(__cplusplus)

typedef void (* rmw_uros_memory_error_handling)(
  char * identifier,
  ucdrBuffer * ucdr,
  size_t size);

/** \addtogroup rmw micro-ROS RMW API
 *  @{
 */

/**
 * \brief Sets the callback functions for handling error in static memory handling
 *
 * \param[in] memory_error_cb callback to be triggered on static memory failure
 */
void rmw_uros_set_memory_error_handling_callback(
  rmw_uros_memory_error_handling memory_error_cb);

/** @}*/

#if defined(__cplusplus)
}
#endif  // if defined(__cplusplus)

#endif  // RMW_MICROROS__MEMORY_ERROR_HANDLING_H_
