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
// limitations under the License

#ifndef RMW_MICROXRCEDDS_C__RMW_UROS_OPTIONS_H
#define RMW_MICROXRCEDDS_C__RMW_UROS_OPTIONS_H

#include <rmw/ret_types.h>
#include <rmw/init_options.h>

/**
 * \brief Parse command line args and fills rmw implementation specific options.
 * `rmw_init_options allocator` is used to allocate the specific rmw options.
 *
 * \param[in] argc Number of arguments.
 * \param[in] argv Arguments.
 * \param[in,out] rmw_options Updated options with rmw specifics.
 * \return RMW_RET_OK If arguments where valid and setted in rmw_init_options.
 * \return RMW_RET_INVALID_ARGUMENT If rmw_init_options is not valid or unexpected arguments.
 */
rmw_ret_t rmw_uros_init_options(int argc, const char* const argv[], rmw_init_options_t* rmw_options);

#endif // !RMW_MICROXRCEDDS_C__RMW_UROS_OPTIONS_H