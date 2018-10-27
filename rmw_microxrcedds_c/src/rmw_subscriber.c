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

#include "./rmw_subscriber.h"  // NOLINT

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rosidl_typesupport_microxrcedds_shared/identifier.h>

#include "./rmw_microxrcedds.h"
#include "./types.h"
#include "./utils.h"
#include "./rmw_microxrcedds_topic.h"

rmw_subscription_t * create_subscriber(
  const rmw_node_t * node, const rosidl_message_type_support_t * type_support,
  const char * topic_name, const rmw_qos_profile_t * qos_policies,
  bool ignore_local_publications)
{
  bool success = false;
  (void)qos_policies;
  (void)ignore_local_publications;

  rmw_subscription_t * rmw_subscriber = (rmw_subscription_t *)rmw_allocate(
    sizeof(rmw_subscription_t));
  rmw_subscriber->data = NULL;
  rmw_subscriber->implementation_identifier = rmw_get_implementation_identifier();
  rmw_subscriber->topic_name = (const char *)(rmw_allocate(sizeof(char) * strlen(topic_name) + 1));
  if (!rmw_subscriber->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto create_subscriber_end;
  }

  CustomNode * custom_node = (CustomNode *)node->data;
  struct Item * memory_node = get_memory(&custom_node->subscription_mem);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available memory node");
    goto create_subscriber_end;
  }

  // TODO(Borja) micro_xrcedds_id is duplicated in subscriber_id and in subscription_gid.data
  CustomSubscription * custom_subscription = (CustomSubscription *)memory_node->data;
  custom_subscription->owner_node = custom_node;
  custom_subscription->subscription_gid.implementation_identifier =
    rmw_get_implementation_identifier();
  custom_subscription->session = &custom_node->session;
  custom_subscription->waiting_for_response = false;

  if ((type_support == get_message_typesupport_handle(type_support,
    ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE)) ||
    (type_support == get_message_typesupport_handle(type_support,
    ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE)))
  {
    custom_subscription->type_support_callbacks =
      (const message_type_support_callbacks_t *)type_support->data;
  }
  if (custom_subscription->type_support_callbacks == NULL) {
    RMW_SET_ERROR_MSG("type support not from this implementation");
    goto create_subscriber_end;
  } else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE) {
    RMW_SET_ERROR_MSG("Not enough memory for impl ids");
    goto create_subscriber_end;
  }

  memset(custom_subscription->subscription_gid.data, 0, RMW_GID_STORAGE_SIZE);
  memcpy(custom_subscription->subscription_gid.data, &custom_subscription->subscriber_id,
    sizeof(uxrObjectId));


  custom_subscription->topic = create_topic(custom_node, topic_name,
      custom_subscription->type_support_callbacks, qos_policies);
  if (custom_subscription->topic == NULL) {
    goto create_subscriber_end;
  }

#ifdef MICRO_XRCEDDS_USE_XML
  char xml_buffer[400];
#elif defined(MICRO_XRCEDDS_USE_REFS)
  char profile_name[64];
#endif

  custom_subscription->subscriber_id = uxr_object_id(custom_node->id_gen++, UXR_SUBSCRIBER_ID);
  uint16_t subscriber_req;
#ifdef MICRO_XRCEDDS_USE_XML
  char subscriber_name[20];
  generate_name(&custom_subscription->subscriber_id, subscriber_name, sizeof(subscriber_name));
  if (!build_subscriber_xml(subscriber_name, xml_buffer, sizeof(xml_buffer))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
    goto create_subscriber_end;
  }
  subscriber_req = uxr_buffer_create_subscriber_xml(&custom_node->session,
      custom_node->reliable_output, custom_subscription->subscriber_id,
      custom_node->participant_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  // TODO(BORJA)  Publisher by reference does not make sense in
  //              current micro XRCE-DDS implementation.
  subscriber_req = uxr_buffer_create_subscriber_xml(&custom_node->session,
      custom_node->reliable_output, custom_subscription->subscriber_id,
      custom_node->participant_id, "", UXR_REPLACE);
#endif


  custom_subscription->datareader_id = uxr_object_id(custom_node->id_gen++, UXR_DATAREADER_ID);
  uint16_t datareader_req;
#ifdef MICRO_XRCEDDS_USE_XML
  if (!build_datareader_xml(topic_name, custom_subscription->type_support_callbacks,
    qos_policies, xml_buffer,
    sizeof(xml_buffer)))
  {
    RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
    goto create_subscriber_end;
  }

  datareader_req = uxr_buffer_create_datareader_xml(&custom_node->session,
      custom_node->reliable_output, custom_subscription->datareader_id,
      custom_subscription->subscriber_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  if (!build_datareader_profile(topic_name, profile_name, sizeof(profile_name))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    goto create_subscriber_end;
  }

  datareader_req = uxr_buffer_create_datareader_ref(&custom_node->session,
      custom_node->reliable_output, custom_subscription->datareader_id,
      custom_subscription->subscriber_id, profile_name, UXR_REPLACE);
#endif

  rmw_subscriber->data = custom_subscription;

  uint16_t requests[] = {subscriber_req, datareader_req};
  uint8_t status[sizeof(requests) / 2];
  if (!uxr_run_session_until_all_status(&custom_node->session, 1000, requests,
    status, sizeof(status)))
  {
    RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
    put_memory(&custom_node->subscription_mem, &custom_subscription->mem);
  }

  success = true;

create_subscriber_end:

  if (!success) {
    rmw_subscription_delete(rmw_subscriber);
    rmw_subscriber = NULL;
  }
  return rmw_subscriber;
}

rmw_ret_t rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
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
  } else if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (strcmp(subscription->implementation_identifier,  // NOLINT
    rmw_get_implementation_identifier()) != 0)
  {
    RMW_SET_ERROR_MSG("subscription handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!subscription->data) {
    RMW_SET_ERROR_MSG("subscription imp is null");
    result_ret = RMW_RET_ERROR;
  } else {
    CustomNode * custom_node = (CustomNode *)node->data;
    CustomSubscription * custom_subscription = (CustomSubscription *)subscription->data;
    uint16_t delete_datareader =
      uxr_buffer_delete_entity(&custom_node->session, custom_node->reliable_output,
        custom_subscription->datareader_id);
    uint16_t delete_subscriber =
      uxr_buffer_delete_entity(&custom_node->session, custom_node->reliable_output,
        custom_subscription->subscriber_id);

    uint16_t requests[] = {delete_datareader, delete_subscriber};
    uint8_t status[sizeof(requests) / 2];
    if (!uxr_run_session_until_all_status(&custom_node->session, 1000, requests, status,
      sizeof(status)))
    {
      RMW_SET_ERROR_MSG("unable to remove publisher from the server");
      result_ret = RMW_RET_ERROR;
    } else {
      rmw_subscription_delete(subscription);
      result_ret = RMW_RET_OK;
    }
  }

  return result_ret;
}
