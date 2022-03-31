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

#include <rmw_microros_internal/utils.h>

#include "./rmw_microros_internal/types.h"
#include "./rmw_microros_internal/error_handling_internal.h"

// TODO(pablogs9) Refactor all this file.

// Static buffers for name generation
char type_buffer_1[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];
char type_buffer_2[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];
char topic_buffer_1[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];
char topic_buffer_2[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];
char node_name_buffer[2 * RMW_UXRCE_NODE_NAME_MAX_LENGTH];

// Prefix strings
static const char ros_topic_prefix[] = "rt";
static const char ros_request_prefix[] = "rq";
static const char ros_reply_prefix[] = "rr";
static const char ros_request_subfix[] = "Request";
static const char ros_reply_subfix[] = "Reply";

bool run_xrce_session(
  rmw_context_impl_t * context,
  uxrStreamId * target_stream,
  uint16_t request,
  int timeout)
{
  if (target_stream->type == UXR_BEST_EFFORT_STREAM) {
    uxr_flash_output_streams(&context->session);
  } else {
    // This only handles one request at time
    uint8_t status;
    if (!uxr_run_session_until_all_status(
        &context->session,
        timeout, &request, &status, 1))
    {
      RMW_UROS_TRACE_MESSAGE("Issues running micro XRCE-DDS session")
      return false;
    }
  }
  return true;
}

uxrQoS_t convert_qos_profile(const rmw_qos_profile_t * rmw_qos)
{
  uxrQoSDurability durability;
  switch (rmw_qos->durability) {
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      durability = UXR_DURABILITY_VOLATILE;
      break;
    case RMW_QOS_POLICY_DURABILITY_UNKNOWN:
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
    default:
      durability = UXR_DURABILITY_TRANSIENT_LOCAL;
      break;
  }

  uxrQoSReliability reliability;
  switch (rmw_qos->reliability) {
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      reliability = UXR_RELIABILITY_BEST_EFFORT;
      break;
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
    default:
      reliability = UXR_RELIABILITY_RELIABLE;
      break;
  }

  uxrQoSHistory history;
  switch (rmw_qos->history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      history = UXR_HISTORY_KEEP_ALL;
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
    default:
      history = UXR_HISTORY_KEEP_LAST;
      break;
  }

  uxrQoS_t qos = {
    .durability = durability,
    .reliability = reliability,
    .history = history,
    .depth = rmw_qos->depth,
  };

  return qos;
}

int generate_service_topics(
  const char * service_name,
  char * request_topic,
  char * reply_topic,
  size_t buffer_size)
{
  snprintf(
    request_topic, buffer_size, "%s%s%s", ros_request_prefix,
    service_name, ros_request_subfix);

  snprintf(
    reply_topic, buffer_size, "%s%s%s", ros_reply_prefix,
    service_name, ros_reply_subfix);

  return 1;
}

int generate_service_types(
  const service_type_support_callbacks_t * members,
  char * request_type,
  char * reply_type,
  size_t buffer_size)
{
  const rosidl_message_type_support_t * req_members = members->request_members_();
  const rosidl_message_type_support_t * res_members = members->response_members_();

  const message_type_support_callbacks_t * req_callbacks =
    (const message_type_support_callbacks_t *)req_members->data;
  const message_type_support_callbacks_t * res_callbacks =
    (const message_type_support_callbacks_t *)res_members->data;

  generate_type_name(req_callbacks, request_type, buffer_size);
  generate_type_name(res_callbacks, reply_type, buffer_size);

  return 0;
}

int generate_name(
  const uxrObjectId * id,
  char name[],
  size_t buffer_size)
{
  static const char format[] = "%hu_%hi";

  int ret = snprintf(name, buffer_size, format, id->id, id->type);

  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
}

size_t generate_type_name(
  const message_type_support_callbacks_t * members,
  char type_name[],
  size_t buffer_size)
{
  static const char * sep = "::";
  static const char * protocol = "dds";
  static const char * suffix = "_";
  size_t full_name_size = strlen(protocol) + strlen(suffix) + strlen(sep) + strlen(
    members->message_name_) + strlen(suffix) +
    ((NULL != members->message_namespace_) ? strlen(members->message_namespace_) : 0) + 1;

  type_name[0] = 0;

  snprintf(
    type_name, buffer_size,
    "%s%s%s%s%s%s%s",
    (NULL != members->message_namespace_) ? members->message_namespace_ : "",
    (NULL != members->message_namespace_) ? sep : "",
    protocol,
    suffix,
    sep,
    members->message_name_,
    suffix
  );

  return full_name_size;
}

int generate_topic_name(
  const char * topic_name,
  char * full_topic_name,
  size_t full_topic_name_size)
{
  int ret = snprintf(
    full_topic_name,
    full_topic_name_size,
    "%s%s",
    ros_topic_prefix,
    topic_name);
  if ((ret < 0) && (ret >= (int)full_topic_name_size)) {
    return 0;
  }
  return ret;
}

bool build_participant_profile(
  char profile_name[],
  size_t buffer_size)
{
  static const char profile[] = "participant_profile";
  bool ret = false;

  if (buffer_size >= sizeof(profile)) {
    memcpy(profile_name, profile, sizeof(profile));
    ret = true;
  }
  return ret;
}

bool build_topic_profile(
  const char * topic_name,
  char profile_name[],
  size_t buffer_size)
{
  const char * const format = "%s__t";

  topic_name++;
  bool ret = false;
  int written = snprintf(profile_name, buffer_size, format, topic_name);
  ret = (written > 0) && (written < (int)buffer_size);
  return ret;
}

bool build_datawriter_profile(
  const char * topic_name,
  char profile_name[],
  size_t buffer_size)
{
  const char * const format = "%s__dw";

  topic_name++;
  bool ret = false;
  int written = snprintf(profile_name, buffer_size, format, topic_name);
  ret = (written > 0) && (written < (int)buffer_size);
  return ret;
}

bool build_datareader_profile(
  const char * topic_name,
  char profile_name[],
  size_t buffer_size)
{
  const char * const format = "%s__dr";

  topic_name++;
  bool ret = false;
  int written = snprintf(profile_name, buffer_size, format, topic_name);
  ret = (written > 0) && (written < (int)buffer_size);
  return ret;
}

bool is_uxrce_rmw_identifier_valid(
  const char * id)
{
  return id != NULL &&
         strcmp(id, rmw_get_implementation_identifier()) == 0;
}
