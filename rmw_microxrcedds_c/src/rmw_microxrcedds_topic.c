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
  struct CustomNode * custom_node,
  const char * topic_name,
  const message_type_support_callbacks_t * message_type_support_callbacks,
  const rmw_qos_profile_t * qos_policies)
{
  // find topic in list
  rmw_uxrce_topic_t * custom_topic_ptr = custom_node->custom_topic_sp;
  while (custom_topic_ptr != NULL) {
    if (message_type_support_callbacks == custom_topic_ptr->message_type_support_callbacks) {
      break;
    }

    custom_topic_ptr = custom_topic_ptr->next_custom_topic;
  }

  // Check if allready exists
  if (custom_topic_ptr != NULL) {
    custom_topic_ptr->usage_account++;
    goto create_topic_end;
  }

  // Generate new memory
  custom_topic_ptr = (rmw_uxrce_topic_t *)rmw_allocate(sizeof(rmw_uxrce_topic_t));
  if (custom_topic_ptr == NULL) {
    RMW_SET_ERROR_MSG("failed to allocate topic interna mem");
    goto create_topic_end;
  }


  // Init
  custom_topic_ptr->sync_with_agent = false;
  custom_topic_ptr->usage_account = 1;
  custom_topic_ptr->owner_node = custom_node;

  // Add to top of the node stack
  custom_topic_ptr->next_custom_topic = custom_node->custom_topic_sp;
  if (custom_node->custom_topic_sp != NULL) {
    custom_node->custom_topic_sp->prev_custom_topic = custom_topic_ptr;
  }
  custom_node->custom_topic_sp = custom_topic_ptr;


  // Asociate to typesupport
  custom_topic_ptr->message_type_support_callbacks = message_type_support_callbacks;


  // Generate topic id
  custom_topic_ptr->topic_id = uxr_object_id(custom_node->id_gen++, UXR_TOPIC_ID);

#ifdef MICRO_XRCEDDS_USE_XML
  char xml_buffer[RMW_UXRCE_XML_BUFFER_LENGTH];
#elif defined(MICRO_XRCEDDS_USE_REFS)
  char profile_name[RMW_UXRCE_REF_BUFFER_LENGTH];
#endif

  // Generate request
  uint16_t topic_req;
#ifdef MICRO_XRCEDDS_USE_XML
  if (!build_topic_xml(topic_name, message_type_support_callbacks,
    qos_policies, xml_buffer, sizeof(xml_buffer)))
  {
    RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
    (void)destroy_topic(custom_topic_ptr);
    custom_topic_ptr = NULL;
    goto create_topic_end;
  }

  topic_req = uxr_buffer_create_topic_xml(&custom_node->session,
      custom_node->reliable_output, custom_topic_ptr->topic_id,
      custom_node->participant_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  (void)qos_policies;
  if (!build_topic_profile(topic_name, profile_name, sizeof(profile_name))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    (void)destroy_topic(custom_topic_ptr);
    custom_topic_ptr = NULL;
    goto create_topic_end;
  }

  topic_req = uxr_buffer_create_topic_ref(&custom_node->session,
      custom_node->reliable_output, custom_topic_ptr->topic_id,
      custom_node->participant_id, profile_name, UXR_REPLACE);
#endif

  // Send the request and wait for response
  uint8_t status;
  custom_topic_ptr->sync_with_agent =
    uxr_run_session_until_all_status(&custom_node->session, 1000, &topic_req,
      &status, 1);
  if (!custom_topic_ptr->sync_with_agent) {
    RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
    (void)destroy_topic(custom_topic_ptr);
    custom_topic_ptr = NULL;
    goto create_topic_end;
  }


create_topic_end:

  return custom_topic_ptr;
}

bool
destroy_topic(rmw_uxrce_topic_t * custom_topic)
{
  bool ok = false;

  if (custom_topic != NULL) {
    custom_topic->usage_account--;
    if (custom_topic->usage_account <= 0) {
      // Remove from the stack
      if (custom_topic->next_custom_topic != NULL) {
        custom_topic->next_custom_topic->prev_custom_topic = custom_topic->prev_custom_topic;
      }
      if (custom_topic->prev_custom_topic != NULL) {
        custom_topic->prev_custom_topic->next_custom_topic = custom_topic->next_custom_topic;
      } else {
        custom_topic->owner_node->custom_topic_sp = NULL;
      }

      if (custom_topic->sync_with_agent) {
        uint16_t request = uxr_buffer_delete_entity(&custom_topic->owner_node->session,
            custom_topic->owner_node->reliable_output,
            custom_topic->topic_id);
        uint8_t status;
        if (!uxr_run_session_until_all_status(&custom_topic->owner_node->session, 1000,
          &request, &status, 1))
        {
          RMW_SET_ERROR_MSG("unable to remove publisher from the server");
        } else {
          ok = true;
        }
      } else {
        ok = true;
      }

      rmw_free(custom_topic);
    } else {
      ok = true;
    }
  }

  return ok;
}


size_t
topic_count(struct CustomNode * custom_node)
{
  size_t count = 0;
  rmw_uxrce_topic_t * custom_topic_ptr = custom_node->custom_topic_sp;
  while (custom_topic_ptr != NULL) {
    count++;
    custom_topic_ptr = custom_topic_ptr->next_custom_topic;
  }

  return count;
}
