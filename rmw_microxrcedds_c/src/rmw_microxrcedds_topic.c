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


custom_topic_t * create_topic(
  struct CustomNode * custom_node,
  const char * topic_name,
  const message_type_support_callbacks_t * message_type_support_callbacks,
  rmw_qos_profile_t * qos_policies)
{
  // find topic in list
  custom_topic_t * custom_topic_ptr = custom_node->custom_topic_sp;
  while (custom_topic_ptr != NULL) {
    if (strcmp(topic_name, custom_topic_ptr->topic_name) == 0) {
      break;
    }
  }

  // Check if allready exists
  if (custom_topic_ptr == NULL) {
    // generate new memory
    custom_topic_ptr = (custom_topic_t *)rmw_allocate(sizeof(custom_topic_t));
    if (custom_topic_ptr == NULL) {
      RMW_SET_ERROR_MSG("failed to allocate topic interna mem");
    } else {
      // Add to top of the node stack
      custom_topic_ptr->next_custom_topic = custom_node->custom_topic_sp;
      if (custom_node->custom_topic_sp != NULL) {
        custom_node->custom_topic_sp->prev_custom_topic = custom_topic_ptr;
      }
      custom_node->custom_topic_sp = custom_topic_ptr;

      // Stote topic name
      int topic_size = sizeof(char) * strlen(topic_name) + 1;
      custom_topic_ptr->topic_name = (char *)(rmw_allocate(topic_size));
      if (custom_topic_ptr->topic_name == NULL) {
        RMW_SET_ERROR_MSG("failed to allocate topic name mem");
        (void)destroy_topic(custom_topic_ptr);
        custom_topic_ptr = NULL;
      } else {
        // Copy the topic name
        if (snprintf(custom_topic_ptr->topic_name, topic_size, "%s", topic_name) != topic_size) {
          RMW_SET_ERROR_MSG("error when storing topic name");
          (void)destroy_topic(custom_topic_ptr);
          custom_topic_ptr = NULL;
        } else {
          // Generate topic id
          custom_topic_ptr->topic_id = uxr_object_id(custom_node->id_gen++, UXR_TOPIC_ID);

          // Generate request
          uint16_t topic_req;

#define MICRO_XRCEDDS_USE_REFS
#ifdef MICRO_XRCEDDS_USE_XML
          char xml_buffer[400];
          if (!build_topic_xml(custom_topic_ptr->topic_name,
            message_type_support_callbacks, qos_policies, xml_buffer, sizeof(xml_buffer)))
          {
            RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
            (void)destroy_topic(custom_topic_ptr);
            custom_topic_ptr = NULL;
          } else {
            topic_req = uxr_buffer_configure_topic_xml(&custom_node->session,
                custom_node->reliable_output, custom_topic_ptr->topic_id,
                custom_node->participant_id, xml_buffer, UXR_REPLACE);
          }
#elif defined(MICRO_XRCEDDS_USE_REFS)
          char profile_name[64];
          if (!build_topic_profile(topic_name, profile_name, sizeof(profile_name))) {
            RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
            (void)destroy_topic(custom_topic_ptr);
            custom_topic_ptr = NULL;
          } else {
            topic_req = uxr_buffer_create_topic_ref(&micro_node->session,
                micro_node->reliable_output, subscription_info->topic_id,
                micro_node->participant_id, profile_name, UXR_REPLACE);
          }
#endif
          if (custom_topic_ptr != NULL) {
            // Send the request and wait for response
            uint8_t status;
            custom_topic_ptr->sync_with_agent =
              uxr_run_session_until_all_status(&custom_node->session, 1000, &topic_req,
                &status, 1);
            if (!custom_topic_ptr->sync_with_agent) {
              RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
              (void)destroy_topic(custom_topic_ptr);
              custom_topic_ptr = NULL;
            }
          }
        }
      }
    }
  }

  return custom_topic_ptr;
}

bool destroy_topic(custom_topic_t * custom_topic)
{
  bool ok = false;

  if (custom_topic != NULL) {
    // Remove from the stack
    if (custom_topic->next_custom_topic != NULL) {
      custom_topic->next_custom_topic->prev_custom_topic = custom_topic->prev_custom_topic;
    }
    if (custom_topic->prev_custom_topic != NULL) {
      custom_topic->prev_custom_topic->next_custom_topic = custom_topic->next_custom_topic;
    }

    // Release memory from the topic name
    if (custom_topic->topic_name != NULL) {
      rmw_free(custom_topic->topic_name);
    }

    // TODO(Javier) Need to find a way to remove the topic on the agent side
    // if (custom_topic_ptr->sync_with_agent) {}

    ok = true;
  }

  return ok;
}
