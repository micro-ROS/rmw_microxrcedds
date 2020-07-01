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

#include "./rmw_microxrcedds_topic.h"  // NOLINT

#include <string.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include "./utils.h"


rmw_uxrce_topic_t *
create_topic(
  struct rmw_uxrce_node_t * custom_node,
  const char * topic_name,
  const message_type_support_callbacks_t * message_type_support_callbacks,
  const rmw_qos_profile_t * qos_policies)
{
  struct rmw_uxrce_mempool_item_t * memory_node = get_memory(&topics_memory);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available memory node");
    goto fail;
  }

  rmw_uxrce_topic_t * custom_topic = (rmw_uxrce_topic_t *)memory_node->data;


  // Init
  custom_topic->sync_with_agent = false;
  custom_topic->usage_account = 1;
  custom_topic->owner_node = custom_node;

  // Asociate to typesupport
  custom_topic->message_type_support_callbacks = message_type_support_callbacks;

  // Generate topic id
  custom_topic->topic_id = uxr_object_id(custom_node->context->id_topic++, UXR_TOPIC_ID);

#ifdef MICRO_XRCEDDS_USE_XML
  char xml_buffer[RMW_UXRCE_XML_BUFFER_LENGTH];
#elif defined(MICRO_XRCEDDS_USE_REFS)
  char profile_name[RMW_UXRCE_REF_BUFFER_LENGTH];
#endif

  // Generate request
  uint16_t topic_req;
#ifdef MICRO_XRCEDDS_USE_XML
  if (!build_topic_xml(
      topic_name, message_type_support_callbacks,
      qos_policies, xml_buffer, sizeof(xml_buffer)))
  {
    RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
    rmw_uxrce_fini_topic_memory(custom_topic);
    custom_topic = NULL;
    goto fail;
  }

  topic_req = uxr_buffer_create_topic_xml(&custom_node->context->session,
      custom_node->context->reliable_output, custom_topic->topic_id,
      custom_node->participant_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  (void)qos_policies;
  if (!build_topic_profile(topic_name, profile_name, sizeof(profile_name))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    rmw_uxrce_fini_topic_memory(custom_topic);
    custom_topic = NULL;
    goto fail;
  }

  topic_req = uxr_buffer_create_topic_ref(&custom_node->context->session,
      custom_node->context->reliable_output, custom_topic->topic_id,
      custom_node->participant_id, profile_name, UXR_REPLACE);
#endif

  // Send the request and wait for response
  uint8_t status;
  custom_topic->sync_with_agent =
    uxr_run_session_until_all_status(&custom_node->context->session, 1000, &topic_req,
      &status, 1);
  if (!custom_topic->sync_with_agent) {
    RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
    rmw_uxrce_fini_topic_memory(custom_topic);
    custom_topic = NULL;
    goto fail;
  }

fail:
  return custom_topic;
}