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

#ifndef RMW_MICROXRCEDDS_TOPIC_H_
#define RMW_MICROXRCEDDS_TOPIC_H_

#include <stddef.h>
#include <uxr/client/client.h>

#include "./types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

CustomTopic *
create_topic(
  struct CustomNode * custom_node,
  const char * topic_name,
  const message_type_support_callbacks_t * message_type_support_callbacks,
  const rmw_qos_profile_t * qos_policies);


bool
destroy_topic(CustomTopic * custom_topic);

size_t
topic_count(struct CustomNode * custom_node);

#if defined(__cplusplus)
}
#endif

#endif  // RMW_MICROXRCEDDS_TOPIC_H_
