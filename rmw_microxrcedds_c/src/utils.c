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

#include <utils.h>

#include <rmw/error_handling.h>

#include "./types.h"


// TODO(pablogs9) Refactor all this file.

static const char ros_topic_prefix[] = "rt";
static const char ros_request_prefix[] = "rq";
static const char ros_reply_prefix[] = "rr";
static const char ros_request_subfix[] = "Request";
static const char ros_reply_subfix[] = "Reply";

bool run_xrce_session(
  rmw_context_impl_t * context,
  uint16_t requests)
{
  if (context->creation_destroy_stream->type == UXR_BEST_EFFORT_STREAM) {
    uxr_flash_output_streams(&context->session);
  } else {
    // This only handles one request at time
    uint8_t status;
    if (!uxr_run_session_until_all_status(
        &context->session,
        RMW_UXRCE_ENTITY_CREATION_DESTROY_TIMEOUT,
        &requests, &status, 1))
    {
      RMW_SET_ERROR_MSG("Issues running micro XRCE-DDS session");
      return false;
    }
  }
  return true;
}

int build_participant_xml(
  size_t domain_id,
  const char * participant_name,
  char xml[],
  size_t buffer_size)
{
  (void)domain_id;
  static const char format[] =
    "<dds>"
    "<participant>"
    "<rtps>"
    "<name>%s</name>"
    "</rtps>"
    "</participant>"
    "</dds>";

  int ret = snprintf(xml, buffer_size, format, participant_name);
  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
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

int build_service_xml(
  const char * service_name_id,
  const char * service_name,
  bool requester,
  const service_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies,
  char xml[],
  size_t buffer_size)
{
  int ret;

  static const char format[] = "<dds>"
    "<%s profile_name=\"%s\" "
    "service_name=\"%s\" "
    "request_type=\"%s\" "
    "reply_type=\"%s\">"
    "<request_topic_name>%s</request_topic_name>"
    "<reply_topic_name>%s</reply_topic_name>"
    "</%s>"
    "</dds>";

  // Retrive request and response types
  const rosidl_message_type_support_t * req_members = members->request_members_();
  const rosidl_message_type_support_t * res_members = members->response_members_();

  const message_type_support_callbacks_t * req_callbacks =
    (const message_type_support_callbacks_t *)req_members->data;
  const message_type_support_callbacks_t * res_callbacks =
    (const message_type_support_callbacks_t *)res_members->data;


  static char req_type_name_buffer[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];
  static char res_type_name_buffer[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];

  generate_type_name(req_callbacks, req_type_name_buffer, RMW_UXRCE_TYPE_NAME_MAX_LENGTH);
  generate_type_name(res_callbacks, res_type_name_buffer, RMW_UXRCE_TYPE_NAME_MAX_LENGTH);

  // Generate request and reply topic names
  char req_full_topic_name[RMW_UXRCE_TOPIC_NAME_MAX_LENGTH + 1 + sizeof(ros_request_prefix) + 1 +
    sizeof(ros_request_subfix)];
  req_full_topic_name[0] = '\0';

  char res_full_topic_name[RMW_UXRCE_TOPIC_NAME_MAX_LENGTH + 1 + sizeof(ros_reply_prefix) + 1 +
    sizeof(ros_reply_subfix)];
  res_full_topic_name[0] = '\0';

  if (!qos_policies->avoid_ros_namespace_conventions) {
    ret = snprintf(
      req_full_topic_name, sizeof(req_full_topic_name), "%s%s%s", ros_request_prefix,
      service_name, ros_request_subfix);
    if ((ret < 0) || (ret >= (int)sizeof(req_full_topic_name))) {
      return 0;
    }

    ret = snprintf(
      res_full_topic_name, sizeof(res_full_topic_name), "%s%s%s", ros_reply_prefix,
      service_name, ros_reply_subfix);
    if ((ret < 0) || (ret >= (int)sizeof(res_full_topic_name))) {
      return 0;
    }
  } else {
    ret = snprintf(req_full_topic_name, sizeof(req_full_topic_name), "%s", service_name);
    if ((ret < 0) || (ret >= (int)sizeof(req_full_topic_name))) {
      return 0;
    }
    ret = snprintf(res_full_topic_name, sizeof(res_full_topic_name), "%s", service_name);
    if ((ret < 0) || (ret >= (int)sizeof(req_full_topic_name))) {
      return 0;
    }
  }


  ret = snprintf(
    xml, buffer_size, format,
    requester ? "requester" : "replier",
    service_name_id,
    service_name,
    req_type_name_buffer,
    res_type_name_buffer,
    req_full_topic_name,
    res_full_topic_name,
    requester ? "requester" : "replier"
  );
  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
}

int build_publisher_xml(
  const char * publisher_name,
  char xml[],
  size_t buffer_size)
{
  (void)publisher_name;
  (void)buffer_size;

  // TODO(pablogs9): Check if there is any case where this xml should be filled for FastDDS
  xml[0] = '\0';
  return 1;
}

int build_subscriber_xml(
  const char * subscriber_name,
  char xml[],
  size_t buffer_size)
{
  (void)subscriber_name;
  (void)buffer_size;

  // TODO(pablogs9): Check if there is any case where this xml should be filled for FastDDS
  xml[0] = '\0';
  return 1;
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

  int ret = 0;
  static char type_name_buffer[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];

  if (RMW_UXRCE_TOPIC_NAME_MAX_LENGTH >= strlen(topic_name) &&
    0 != generate_type_name(members, type_name_buffer, sizeof(type_name_buffer)))
  {
    char full_topic_name[RMW_UXRCE_TOPIC_NAME_MAX_LENGTH + 1 + sizeof(ros_topic_prefix)];

    if (!qos_policies->avoid_ros_namespace_conventions) {
      ret = snprintf(
        full_topic_name, sizeof(full_topic_name), "%s%s", ros_topic_prefix,
        topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    } else {
      ret = snprintf(full_topic_name, sizeof(full_topic_name), "%s", topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    }

    ret = snprintf(xml, buffer_size, format, full_topic_name, type_name_buffer);
    if ((ret < 0) && (ret >= (int)buffer_size)) {
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
  static char type_name_buffer[RMW_UXRCE_TYPE_NAME_MAX_LENGTH];

  if (0 != generate_type_name(members, type_name_buffer, sizeof(type_name_buffer))) {
    char full_topic_name[RMW_UXRCE_TOPIC_NAME_MAX_LENGTH + 1 + sizeof(ros_topic_prefix)];
    full_topic_name[0] = '\0';

    if (!qos_policies->avoid_ros_namespace_conventions) {
      ret = snprintf(
        full_topic_name, sizeof(full_topic_name), "%s%s", ros_topic_prefix,
        topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    } else {
      ret = snprintf(full_topic_name, sizeof(full_topic_name), "%s", topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    }

    ret = snprintf(
      xml,
      buffer_size,
      format,
      (qos_policies->reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
      "BEST_EFFORT" : "RELIABLE",
      full_topic_name,
      type_name_buffer);

    if ((ret < 0) && (ret >= (int)buffer_size)) {
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
    "<historyMemoryPolicy>PREALLOCATED_WITH_REALLOC</historyMemoryPolicy>"
    "<qos>"
    "<reliability>"
    "<kind>%s</kind>"
    "</reliability>"
    "</qos>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>%s</name>"
    "<dataType>%s</dataType>"
    "<historyQos>"
    "<kind>KEEP_ALL</kind>"
    "</historyQos>"
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
    "<historyMemoryPolicy>PREALLOCATED_WITH_REALLOC</historyMemoryPolicy>"
    "<qos>"
    "<reliability>"
    "<kind>%s</kind>"
    "</reliability>"
    "</qos>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>%s</name>"
    "<dataType>%s</dataType>"
    "<historyQos>"
    "<kind>KEEP_ALL</kind>"
    "</historyQos>"
    "</topic>"
    "</data_reader>"
    "</dds>";

  return build_xml(format, topic_name, members, qos_policies, xml, buffer_size);
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
