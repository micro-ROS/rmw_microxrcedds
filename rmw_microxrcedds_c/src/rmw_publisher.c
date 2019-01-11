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

#include "./rmw_publisher.h"  // NOLINT

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>
#include <rosidl_typesupport_microxrcedds_shared/identifier.h>
#include <rosidl_typesupport_microxrcedds_shared/message_type_support.h>

#include "./rmw_microxrcedds.h"
#include "./rmw_node.h"
#include "./types.h"
#include "./utils.h"
#include "./rmw_microxrcedds_topic.h"

rmw_publisher_t * create_publisher(
  const rmw_node_t * node, const rosidl_message_type_support_t * type_supports,
  const char * topic_name, const rmw_qos_profile_t * qos_policies)
{
  bool success = false;

  rmw_publisher_t * rmw_publisher = (rmw_publisher_t *)rmw_allocate(sizeof(rmw_publisher_t));
  rmw_publisher->data = NULL;
  rmw_publisher->implementation_identifier = rmw_get_implementation_identifier();
  rmw_publisher->topic_name = (const char *)(rmw_allocate(sizeof(char) * strlen(topic_name) + 1));
  if (!rmw_publisher->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto create_publisher_end;
  }

  CustomNode * custom_node = (CustomNode *)node->data;
  struct Item * memory_node = get_memory(&custom_node->publisher_mem);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available memory node");
    goto create_publisher_end;
  }

  // TODO(Borja) micro_xrcedds_id is duplicated in publisher_id and in publisher_gid.data
  CustomPublisher * custom_publisher = (CustomPublisher *)memory_node->data;
  custom_publisher->owner_node = custom_node;
  custom_publisher->publisher_gid.implementation_identifier = rmw_get_implementation_identifier();
  custom_publisher->session = &custom_node->session;


  const rosidl_message_type_support_t * type_support = get_message_typesupport_handle(
    type_supports, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);
  if (!type_support) {
    type_support = get_message_typesupport_handle(
      type_supports, ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE);
    if (!type_support) {
      RMW_SET_ERROR_MSG("type support not from this implementation");
      goto create_publisher_end;
    }
  }

  custom_publisher->type_support_callbacks =
    (const message_type_support_callbacks_t *)type_support->data;

  if (custom_publisher->type_support_callbacks == NULL) {
    RMW_SET_ERROR_MSG("type support data is NULL");
    goto create_publisher_end;
  } else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE) {
    RMW_SET_ERROR_MSG("Not enough memory for impl ids");
    goto create_publisher_end;
  }

  memset(custom_publisher->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
  memcpy(custom_publisher->publisher_gid.data, &custom_publisher->publisher_id,
    sizeof(uxrObjectId));

  custom_publisher->topic = create_topic(custom_node, topic_name,
      custom_publisher->type_support_callbacks, qos_policies);
  if (custom_publisher->topic == NULL) {
    goto create_publisher_end;
  }

#ifdef MICRO_XRCEDDS_USE_XML
  char xml_buffer[400];
#elif defined(MICRO_XRCEDDS_USE_REFS)
  char profile_name[64];
#endif

  custom_publisher->publisher_id = uxr_object_id(custom_node->id_gen++, UXR_PUBLISHER_ID);
  uint16_t publisher_req;
#ifdef MICRO_XRCEDDS_USE_XML
  char publisher_name[20];
  generate_name(&custom_publisher->publisher_id, publisher_name, sizeof(publisher_name));
  if (!build_publisher_xml(publisher_name, xml_buffer, sizeof(xml_buffer))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for publisher creation");
    goto create_publisher_end;
  }
  publisher_req = uxr_buffer_create_publisher_xml(custom_publisher->session,
      custom_node->reliable_output, custom_publisher->publisher_id,
      custom_node->participant_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  // TODO(BORJA) Publisher by reference does not make sense
  //             in current micro XRCE-DDS implementation.
  publisher_req = uxr_buffer_create_publisher_xml(custom_publisher->session,
      custom_node->reliable_output, custom_publisher->publisher_id,
      custom_node->participant_id, "", UXR_REPLACE);
#endif

  custom_publisher->datawriter_id = uxr_object_id(custom_node->id_gen++, UXR_DATAWRITER_ID);
  uint16_t datawriter_req;
#ifdef MICRO_XRCEDDS_USE_XML
  if (!build_datawriter_xml(topic_name, custom_publisher->type_support_callbacks,
    qos_policies, xml_buffer, sizeof(xml_buffer)))
  {
    RMW_SET_ERROR_MSG("failed to generate xml request for publisher creation");
    goto create_publisher_end;
  }

  datawriter_req = uxr_buffer_create_datawriter_xml(
    custom_publisher->session, custom_node->reliable_output, custom_publisher->datawriter_id,
    custom_publisher->publisher_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  if (!build_datawriter_profile(topic_name, profile_name, sizeof(profile_name))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    goto create_publisher_end;
  }

  datawriter_req = uxr_buffer_create_datawriter_ref(custom_publisher->session,
      custom_node->reliable_output, custom_publisher->datawriter_id,
      custom_publisher->publisher_id, profile_name, UXR_REPLACE);
#endif

  rmw_publisher->data = custom_publisher;

  uint16_t requests[] = {publisher_req, datawriter_req};
  uint8_t status[sizeof(requests) / 2];
  if (!uxr_run_session_until_all_status(custom_publisher->session, 1000, requests,
    status, sizeof(status)))
  {
    RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
    goto create_publisher_end;
  }

  success = true;

create_publisher_end:

  if (!success) {
    rmw_publisher_delete(rmw_publisher);
    rmw_publisher = NULL;
  }
  return rmw_publisher;
}

rmw_ret_t rmw_destroy_publisher(rmw_node_t * node, rmw_publisher_t * publisher)
{
  EPROS_PRINT_TRACE()
  rmw_ret_t result_ret = RMW_RET_OK;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!node->data) {
    RMW_SET_ERROR_MSG("node imp is null");
    result_ret = RMW_RET_ERROR;
  } else if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (strcmp(publisher->implementation_identifier,  // NOLINT
    rmw_get_implementation_identifier()) != 0)
  {
    RMW_SET_ERROR_MSG("publisher handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!publisher->data) {
    RMW_SET_ERROR_MSG("publisher imp is null");
    result_ret = RMW_RET_ERROR;
  } else {
    CustomNode * custom_node = (CustomNode *)node->data;
    CustomPublisher * custom_publisher = (CustomPublisher *)publisher->data;
    uint16_t delete_writer = uxr_buffer_delete_entity(custom_publisher->session,
        custom_publisher->owner_node->reliable_output,
        custom_publisher->datawriter_id);
    uint16_t delete_publisher = uxr_buffer_delete_entity(
      custom_publisher->session, custom_publisher->owner_node->reliable_output,
      custom_publisher->publisher_id);

    uint16_t requests[] = {delete_writer, delete_publisher};
    uint8_t status[sizeof(requests) / 2];
    if (!uxr_run_session_until_all_status(custom_publisher->session, 1000, requests, status,
      sizeof(status)))
    {
      RMW_SET_ERROR_MSG("unable to remove publisher from the server");
      result_ret = RMW_RET_ERROR;
    } else {
      rmw_publisher_delete(publisher);
      result_ret = RMW_RET_OK;
    }
  }

  return result_ret;
}

rmw_ret_t rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  EPROS_PRINT_TRACE()
  rmw_ret_t ret = RMW_RET_OK;
  if (!publisher) {
    RMW_SET_ERROR_MSG("publisher pointer is null");
    ret = RMW_RET_ERROR;
  } else if (!ros_message) {
    RMW_SET_ERROR_MSG("ros_message pointer is null");
    ret = RMW_RET_ERROR;
  }
  if (strcmp(publisher->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("publisher handle not from this implementation");
    ret = RMW_RET_ERROR;
  } else if (!publisher->data) {
    RMW_SET_ERROR_MSG("publisher imp is null");
    ret = RMW_RET_ERROR;
  } else {
    CustomPublisher * custom_publisher = (CustomPublisher *)publisher->data;
    const message_type_support_callbacks_t * functions = custom_publisher->type_support_callbacks;
    bool written = true;
    uint32_t topic_length = functions->get_serialized_size(ros_message);
    uint32_t payload_length = 0;
    payload_length = (uint16_t)(payload_length + 4);  // request_id + object_id
    payload_length = (uint16_t)(payload_length + 4);  // request_id + object_id

    ucdrBuffer mb;
    if (uxr_prepare_output_stream(custom_publisher->session,
      custom_publisher->owner_node->reliable_output, custom_publisher->datawriter_id, &mb,
      topic_length))
    {
      ucdrBuffer mb_topic;
      ucdr_init_buffer(&mb_topic, mb.iterator, topic_length);
      written &= functions->cdr_serialize(ros_message, &mb_topic);

      written &= uxr_run_session_until_confirm_delivery(custom_publisher->session, 1000);
    }
    if (!written) {
      RMW_SET_ERROR_MSG("error publishing message");
      ret = RMW_RET_ERROR;
    }
  }
  return ret;
}
