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

#ifndef UTILS_H_
#define UTILS_H_

#include <rmw/rmw.h>

#include "./types.h"


// (Borja) decide wat to do with this macro.
#define EPROS_PRINT_TRACE() ;  // printf("func %s, in file %s:%d\n", __func__, __FILE__, __LINE__);

void rmw_delete(void * rmw_allocated_ptr);
void rmw_node_delete(rmw_node_t * node);
void rmw_publisher_delete(rmw_publisher_t * publisher);
void rmw_subscription_delete(rmw_subscription_t * subscriber);
void rmw_client_delete(rmw_client_t * client);

void customnode_clear(CustomNode * node);

int generate_name(const uxrObjectId * id, char name[], size_t buffer_size);
size_t generate_type_name(
  const message_type_support_callbacks_t * members, char type_name[],
  size_t buffer_size);

int build_service_xml(const char * service_name_id, const char * service_name, bool requester,  const service_type_support_callbacks_t * members,
 const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size);
 
int build_participant_xml(
  size_t domain_id, const char * participant_name, char xml[],
  size_t buffer_size);
int build_publisher_xml(const char * publisher_name, char xml[], size_t buffer_size);
int build_subscriber_xml(const char * subscriber_name, char xml[], size_t buffer_size);
int build_topic_xml(
  const char * topic_name, const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size);
int build_datawriter_xml(
  const char * topic_name, const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size);
int build_datareader_xml(
  const char * topic_name, const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size);

bool build_participant_profile(char profile_name[], size_t buffer_size);
bool build_topic_profile(const char * topic_name, char profile_name[], size_t buffer_size);
bool build_datawriter_profile(const char * topic_name, char profile_name[], size_t buffer_size);
bool build_datareader_profile(const char * topic_name, char profile_name[], size_t buffer_size);

#endif  // UTILS_H_
