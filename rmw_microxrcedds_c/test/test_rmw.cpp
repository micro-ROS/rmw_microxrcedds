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

#include <gtest/gtest.h>

#include <rmw/rmw.h>
#include <rmw/init_options.h>
#include <rcutils/allocator.h>

#include <rmw_uros/options.h>

/*
   Testing rmw init and shutdown. htps://github.com/microROS/rmw-microxrcedds/issues/14
*/
TEST(rmw_microxrcedds, init_shutdown) {
  rmw_context_t test_context = rmw_get_zero_initialized_context();
  rmw_init_options_t test_options = rmw_get_zero_initialized_init_options();
  ASSERT_EQ(rmw_init_options_init(&test_options, rcutils_get_default_allocator()), RMW_RET_OK);
  ASSERT_EQ(rmw_init(&test_options, &test_context), RMW_RET_OK);
  ASSERT_EQ(rmw_shutdown(&test_context), RMW_RET_OK);
}

/*
   Testing rmw agent autodiscovery.
*/
TEST(rmw_microxrcedds, autodiscover) {
  rmw_context_t test_context = rmw_get_zero_initialized_context();
  rmw_init_options_t test_options = rmw_get_zero_initialized_init_options();
  ASSERT_EQ(rmw_init_options_init(&test_options, rcutils_get_default_allocator()), RMW_RET_OK);
  ASSERT_EQ(rmw_uros_discover_agent(&test_options), RMW_RET_OK);
  ASSERT_EQ(rmw_init(&test_options, &test_context), RMW_RET_OK);
  ASSERT_EQ(rmw_shutdown(&test_context), RMW_RET_OK);
}

/*
   Testing rmw agent ping.
*/
TEST(rmw_microxrcedds, agent_ping) {
  rmw_context_t test_context = rmw_get_zero_initialized_context();
  rmw_init_options_t test_options = rmw_get_zero_initialized_init_options();
  ASSERT_EQ(rmw_init_options_init(&test_options, rcutils_get_default_allocator()), RMW_RET_OK);
  ASSERT_EQ(rmw_init(&test_options, &test_context), RMW_RET_OK);

  ASSERT_EQ(rmw_uros_ping_agent(100, 1), RMW_RET_OK);

  ASSERT_EQ(rmw_shutdown(&test_context), RMW_RET_OK);

  ASSERT_EQ(rmw_uros_ping_agent(100, 1), RMW_RET_OK);
}
