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

#include <rmw/allocators.h>

#include "./utils.h"

static const char ros_topic_prefix[] = "rt";

void custompublisher_clear(CustomPublisher * publisher);
void customsubscription_clear(CustomSubscription * subscription);

void rmw_delete(void * rmw_allocated_ptr)
{
  rmw_free(rmw_allocated_ptr);
  rmw_allocated_ptr = NULL;
}

void rmw_node_delete(rmw_node_t * node)
{
  if (node->namespace_) {
    rmw_delete((char *)node->namespace_);
  }
  if (node->name) {
    rmw_delete((char *)node->name);
  }
  if (node->implementation_identifier) {
    node->implementation_identifier = NULL;
  }
  if (node->data) {
    customnode_clear((CustomNode *)node->data);
    node->data = NULL;
  }

  rmw_node_free(node);
  node = NULL;
}

void rmw_publisher_delete(rmw_publisher_t * publisher)
{
  if (publisher->implementation_identifier) {
    publisher->implementation_identifier = NULL;
  }
  if (publisher->topic_name) {
    rmw_delete((char *)publisher->topic_name);
  }
  if (publisher->data) {
    custompublisher_clear((CustomPublisher *)publisher->data);
    publisher->data = NULL;
  }
  rmw_delete(publisher);
}

void custompublisher_clear(CustomPublisher * publisher)
{
  if (publisher) {
    memset(&publisher->publisher_id, 0, sizeof(uxrObjectId));
    memset(&publisher->datawriter_id, 0, sizeof(uxrObjectId));
    memset(&publisher->topic_id, 0, sizeof(uxrObjectId));
    publisher->publisher_gid.implementation_identifier = NULL;
    memset(&publisher->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
    publisher->type_support_callbacks = NULL;
  }
}

void publishers_clear(CustomPublisher publishers[MAX_PUBLISHERS_X_NODE])
{
  for (size_t i = 0; i < MAX_PUBLISHERS_X_NODE; i++) {
    custompublisher_clear(&publishers[i]);
  }
}

void rmw_subscription_delete(rmw_subscription_t * subscriber)
{
  if (subscriber->implementation_identifier) {
    subscriber->implementation_identifier = NULL;
  }
  if (subscriber->topic_name) {
    rmw_delete((char *)subscriber->topic_name);
  }
  if (subscriber->data) {
    customsubscription_clear((CustomSubscription *)subscriber->data);
    subscriber->data = NULL;
  }
  rmw_delete(subscriber);
}

void customsubscription_clear(CustomSubscription * subscription)
{
  if (subscription) {
    memset(&subscription->subscriber_id, 0, sizeof(uxrObjectId));
    memset(&subscription->datareader_id, 0, sizeof(uxrObjectId));
    memset(&subscription->topic_id, 0, sizeof(uxrObjectId));
    subscription->subscription_gid.implementation_identifier = NULL;
    memset(&subscription->subscription_gid.data, 0, RMW_GID_STORAGE_SIZE);
    subscription->type_support_callbacks = NULL;
  }
}

void subscriptions_clear(CustomSubscription subscriptions[MAX_SUBSCRIPTIONS_X_NODE])
{
  for (size_t i = 0; i < MAX_SUBSCRIPTIONS_X_NODE; i++) {
    customsubscription_clear(&subscriptions[i]);
  }
}

void customnode_clear(CustomNode * node)
{
  if (node) {
    publishers_clear(node->publisher_info);
    free_mem_pool(&node->publisher_mem);
    subscriptions_clear(node->subscription_info);
    free_mem_pool(&node->subscription_mem);
  }
}

int build_participant_xml(
  size_t domain_id,
  const char * participant_name,
  char xml[],
  size_t buffer_size)
{
    static const char format[] =
        "<dds>"
        // "<participant profile_name=\"participant_profile\">"
        "<participant>"
        "<rtps>"
        "<name>%s</name>"
        "</rtps>"
        "</participant>"
        "</dds>";
    int ret = 0;
    if (buffer_size >= (sizeof(format) - 5 + strlen(participant_name) + sizeof(domain_id)))
    {
        ret = sprintf(xml, format, participant_name);
    }

    /*
    int ret = snprintf(xml, buffer_size, format, domain_id, participant_name);
    if (ret >= (int)buffer_size) {
       ret = 0;
    }
    */

    return ret;
}

int build_publisher_xml(
  const char * publisher_name,
  char xml[],
  size_t buffer_size)
{
  static const char format[] = "<publisher name=\"%s\">";

  int ret = snprintf(xml, buffer_size, format, publisher_name);
  if (ret >= (int)buffer_size) {
    ret = 0;
  }

  return ret;
}

int build_subscriber_xml(
  const char * subscriber_name,
  char xml[],
  size_t buffer_size)
{
  static const char format[] = "<subscriber name=\"%s\">";

  int ret = snprintf(xml, buffer_size, format, subscriber_name);
  if (ret >= (int)buffer_size) {
    ret = 0;
  }

  return ret;
}

int generate_name(
  const uxrObjectId * id,
  char name[],
  size_t buffer_size)
{
  static const char format[] = "%hu_%hi";

  int ret = snprintf(name, buffer_size, format, id->id, id->type);
  if (ret >= (int)buffer_size) {
    ret = 0;
  }

  return ret;
}

int generate_type_name(
  const message_type_support_callbacks_t * members,
  const char * sep,
  char type_name[],
  size_t buffer_size)
{
  static const char format[] = "%s::%s::dds_::%s_";

  int ret = snprintf(type_name, buffer_size, format,
      members->package_name_, sep, members->message_name_);

  if (ret >= (int)buffer_size) {
    ret = 0;
  }

  return ret;
}

int build_topic_xml(
  const char * topic_name,
  const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies,
  char xml[],
  size_t buffer_size)
{
    static const char format[] = 
        "<dds>"
        "<topic>"
        "<name>%s</name>"
        "<dataType>%s</dataType>"
        "</topic>"
        "</dds>";

    int ret                    = 0;
    static char type_name_buffer[50];
    if (RMW_TOPIC_NAME_MAX_NAME_LENGTH >= strlen(topic_name) &&
        generate_type_name(members, "msg", type_name_buffer, sizeof(type_name_buffer)))
    {
        char full_topic_name[RMW_TOPIC_NAME_MAX_NAME_LENGTH + 1 + sizeof(ros_topic_prefix)];
        full_topic_name[0] = '\0'; // Clear Cstring for strcat function.
        if (!qos_policies->avoid_ros_namespace_conventions)
        {
            strcat(full_topic_name, ros_topic_prefix);
        }
        strcat(full_topic_name, topic_name);

        if (buffer_size >= (sizeof(format) - 4 + strlen(full_topic_name) + strlen(type_name_buffer)))
        {
            ret = sprintf(xml, format, full_topic_name, type_name_buffer);
        }

        /*
        if (!qos_policies->avoid_ros_namespace_conventions) {
        if (snprintf(full_topic_name, sizeof(full_topic_name), "%s%s", ros_topic_prefix,
          topic_name) >=
          (int)sizeof(full_topic_name))
        {
          return 0;
        }
      } else {
        if (snprintf(full_topic_name, sizeof(full_topic_name), "%s", topic_name) >=
          (int)sizeof(full_topic_name))
        {
          return 0;
        }
        */
    }

    ret = snprintf(xml, buffer_size, format, full_topic_name, type_name_buffer);
    if (ret >= (int)buffer_size) {
      ret = 0;
    }
  }

  return ret;
}

int build_xml(
  const char * format,
  const char * topic_name,
  const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies,
  char xml[],
  size_t buffer_size)
{
  int ret = 0;
  static char type_name_buffer[50];

  if (generate_type_name(members, "msg", type_name_buffer, sizeof(type_name_buffer))) {
    char full_topic_name[RMW_TOPIC_NAME_MAX_NAME_LENGTH + 1 + sizeof(ros_topic_prefix)];
    full_topic_name[0] = '\0';

    if (!qos_policies->avoid_ros_namespace_conventions) {
      if (snprintf(full_topic_name, sizeof(full_topic_name), "%s%s", ros_topic_prefix,
        topic_name) >=
        (int)sizeof(full_topic_name))
      {
        return 0;
      }
    } else {
      if (snprintf(full_topic_name, sizeof(full_topic_name), "%s", topic_name) >=
        (int)sizeof(full_topic_name))
      {
        return 0;
      }
    }

    ret = snprintf(xml, buffer_size, format, full_topic_name, type_name_buffer);
    if (ret >= (int)buffer_size) {
      ret = 0;
    }
  }
  return ret;
}

int build_datawriter_xml(
  const char * topic_name,
  const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies,
  char xml[],
  size_t buffer_size)
{
    static const char format[] =
        "<dds>"
        "<data_writer>"
        "<topic>"
        "<kind>NO_KEY</kind>"
        "<name>%s</name>"
        "<dataType>%s</dataType>"
        "</topic>"
        "</data_writer>"
        "</dds>";
    return build_xml(format, topic_name, members, qos_policies, xml, buffer_size);
}

int build_datareader_xml(
  const char * topic_name,
  const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies,
  char xml[],
  size_t buffer_size)
{
    static const char format[] =
        "<dds>"
        "<data_reader>"
        "<topic>"
        "<kind>NO_KEY</kind>"
        "<name>%s</name>"
        "<dataType>%s</dataType>"
        "</topic>"
        "</data_reader>"
        "</dds>";
    return build_xml(format, topic_name, members, qos_policies, xml, buffer_size);
}

bool build_participant_profile(char profile_name[], size_t buffer_size)
{
  static const char profile[] = "participant_profile";
  bool ret = false;

  if (buffer_size >= sizeof(profile)) {
    memcpy(profile_name, profile, sizeof(profile));
    ret = true;
  }

  return ret;
}

bool build_topic_profile(const char * topic_name, char profile_name[], size_t buffer_size)
{
  const char * const format = "%s_t";
  topic_name++;

  return snprintf(profile_name, buffer_size, format, topic_name) >= (int)buffer_size;
}

bool build_datawriter_profile(const char * topic_name, char profile_name[], size_t buffer_size)
{
  const char * const format = "%s_p";
  topic_name++;

  return snprintf(profile_name, buffer_size, format, topic_name) >= (int)buffer_size;
}

bool build_datareader_profile(const char * topic_name, char profile_name[], size_t buffer_size)
{
  const char * const format = "%s_s";
  topic_name++;

  return snprintf(profile_name, buffer_size, format, topic_name) >= (int)buffer_size;
}
