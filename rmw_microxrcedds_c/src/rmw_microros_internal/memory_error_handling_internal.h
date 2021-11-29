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

#ifndef RMW_MICROROS_INTERNAL__ERROR_HANDLING_INTERNAL_H_
#define RMW_MICROROS_INTERNAL__ERROR_HANDLING_INTERNAL_H_

#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/error_handling.h>

#if defined(__cplusplus)
extern "C"
{
#endif  // if defined(__cplusplus)

#ifdef RMW_UROS_ERROR_HANDLING
extern rmw_uros_error_handling error_callback;

// #define RMW_UROS_TRACE_ERROR(source, context) if(NULL != error_callback) {error_callback(source, (rmw_uros_error_context_t) context);}
#define RMW_UROS_TRACE_ERROR(entity, source, ...) if (NULL != error_callback) {error_callback( \
      entity, source, (rmw_uros_error_context_t) {__VA_ARGS__});}

#else
#define RMW_UROS_TRACE_ERROR(source, context)
#endif  // RMW_UROS_ERROR_HANDLING

#if defined(__cplusplus)
}
#endif  // if defined(__cplusplus)

#endif  // RMW_MICROROS_INTERNAL__ERROR_HANDLING_INTERNAL_H_
