// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <rmw_publisher.h>

#include <rmw_microxrcedds_c/config.h>

#ifdef RMW_UXRCE_GRAPH
#include <rmw/get_topic_endpoint_info.h>
#endif  // RMW_UXRCE_GRAPH

#ifdef HAVE_C_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#endif /* ifdef HAVE_C_TYPESUPPORT */
#ifdef HAVE_CPP_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_cpp/identifier.h>
#endif /* ifdef HAVE_CPP_TYPESUPPORT */
#include <rosidl_typesupport_microxrcedds_c/message_type_support.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

#include "./utils.h"
#include "./rmw_microxrcedds_topic.h"

rmw_ret_t
rmw_init_publisher_allocation(
  const rosidl_message_type_support_t * type_support,
  const rosidl_runtime_c__Sequence__bound * message_bounds,
  rmw_publisher_allocation_t * allocation)
{
  (void)type_support;
  (void)message_bounds;
  (void)allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_fini_publisher_allocation(
  rmw_publisher_allocation_t * allocation)
{
  (void)allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_policies,
  const rmw_publisher_options_t * publisher_options)
{
  (void)publisher_options;

  rmw_publisher_t * rmw_publisher = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier)) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("publisher topic is null or empty string");
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
  } else {
    rmw_publisher = (rmw_publisher_t *)rmw_allocate(sizeof(rmw_publisher_t));
    rmw_publisher->data = NULL;
    rmw_publisher->implementation_identifier = rmw_get_implementation_identifier();
    size_t topic_name_size = strlen(topic_name) + 1;
    rmw_publisher->topic_name =
      (const char *)(rmw_allocate(sizeof(char) * topic_name_size));
    if (!rmw_publisher->topic_name) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }
    snprintf((char *)rmw_publisher->topic_name, topic_name_size, "%s", topic_name);

    rmw_uxrce_node_t * custom_node = (rmw_uxrce_node_t *)node->data;
    rmw_uxrce_mempool_item_t * memory_node = get_memory(&publisher_memory);
    if (!memory_node) {
      RMW_SET_ERROR_MSG("Not available memory node");
      goto fail;
    }

    rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)memory_node->data;
    custom_publisher->rmw_handle = rmw_publisher;
    custom_publisher->owner_node = custom_node;
    memcpy(&custom_publisher->qos, qos_policies, sizeof(rmw_qos_profile_t));

    custom_publisher->stream_id =
      (qos_policies->reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
      custom_node->context->best_effort_output :
      custom_node->context->reliable_output;

    custom_publisher->cs_cb_size = NULL;
    custom_publisher->cs_cb_serialization = NULL;

    const rosidl_message_type_support_t * type_support_xrce = NULL;
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE
    type_support_xrce = get_message_typesupport_handle(
      type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);
#endif /* ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE */
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE
    if (NULL == type_support_xrce) {
      type_support_xrce = get_message_typesupport_handle(
        type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE);
    }
#endif /* ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE */
    if (NULL == type_support_xrce) {
      RMW_SET_ERROR_MSG("Undefined type support");
      goto fail;
    }

    custom_publisher->type_support_callbacks =
      (const message_type_support_callbacks_t *)type_support_xrce->data;

    if (custom_publisher->type_support_callbacks == NULL) {
      RMW_SET_ERROR_MSG("type support data is NULL");
      goto fail;
    }

    // Create topic
    custom_publisher->topic = create_topic(
      custom_node, topic_name,
      custom_publisher->type_support_callbacks, qos_policies);

    if (custom_publisher->topic == NULL) {
      RMW_SET_ERROR_MSG("Error creating topic");
      goto fail;
    }

    // Create publisher
    custom_publisher->publisher_id = uxr_object_id(
      custom_node->context->id_publisher++,
      UXR_PUBLISHER_ID);
    uint16_t publisher_req = UXR_INVALID_REQUEST_ID;

  #ifdef RMW_UXRCE_USE_REFS
    publisher_req = uxr_buffer_create_publisher_xml(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->creation_destroy_stream,
      custom_publisher->publisher_id,
      custom_node->participant_id, "", UXR_REPLACE);
  #else
    publisher_req = uxr_buffer_create_publisher_bin(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->creation_destroy_stream,
      custom_publisher->publisher_id,
      custom_node->participant_id,
      UXR_REPLACE);
  #endif /* ifdef RMW_UXRCE_USE_REFS */

    if (!run_xrce_session(custom_node->context, publisher_req)) {
      put_memory(&publisher_memory, &custom_publisher->mem);
      goto fail;
    }

    rmw_publisher->data = custom_publisher;

    // Create datawriter
    custom_publisher->datawriter_id = uxr_object_id(
      custom_node->context->id_datawriter++,
      UXR_DATAWRITER_ID);
    uint16_t datawriter_req = UXR_INVALID_REQUEST_ID;

  #ifdef RMW_UXRCE_USE_REFS
    if (!build_datawriter_profile(
        topic_name, rmw_uxrce_entity_naming_buffer,
        sizeof(rmw_uxrce_entity_naming_buffer)))
    {
      RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
      goto fail;
    }

    datawriter_req = uxr_buffer_create_datawriter_ref(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->creation_destroy_stream,
      custom_publisher->datawriter_id,
      custom_publisher->publisher_id, rmw_uxrce_entity_naming_buffer, UXR_REPLACE);
  #else
    bool reliability = qos_policies->reliability == RMW_QOS_POLICY_RELIABILITY_RELIABLE ||
      qos_policies->reliability == RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT;
    bool history = qos_policies->history == RMW_QOS_POLICY_HISTORY_KEEP_LAST;

    uxrQoSDurability durability;
    switch (qos_policies->durability) {
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

    datawriter_req = uxr_buffer_create_datawriter_bin(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->creation_destroy_stream,
      custom_publisher->datawriter_id,
      custom_publisher->publisher_id,
      custom_publisher->topic->topic_id,
      reliability,
      history,
      durability,
      UXR_REPLACE);
  #endif /* ifdef RMW_UXRCE_USE_REFS */

    if (!run_xrce_session(custom_node->context, datawriter_req)) {
      put_memory(&publisher_memory, &custom_publisher->mem);
      goto fail;
    }
  }

  return rmw_publisher;
fail:
  rmw_uxrce_fini_publisher_memory(rmw_publisher);
  rmw_publisher = NULL;
  return rmw_publisher;
}

rmw_ret_t
rmw_publisher_count_matched_subscriptions(
  const rmw_publisher_t * publisher,
  size_t * subscription_count)
{
#ifdef RMW_UXRCE_GRAPH
  rmw_ret_t ret = RMW_RET_OK;
  const char * topic_name = publisher->topic_name;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;
  const rmw_node_t * node = custom_publisher->owner_node->rmw_handle;

  rmw_topic_endpoint_info_array_t subscriptions_info =
    rmw_get_zero_initialized_topic_endpoint_info_array();

  if (RMW_RET_OK != rmw_get_subscriptions_info_by_topic(
      node,
      &allocator,
      topic_name,
      false,
      &subscriptions_info))
  {
    ret = RMW_RET_ERROR;
    goto pub_count_sub_fail;
  } else {
    *subscription_count = subscriptions_info.size;
  }

pub_count_sub_fail:
  if (RMW_RET_OK != rmw_topic_endpoint_info_array_fini(&subscriptions_info, &allocator)) {
    ret = RMW_RET_ERROR;
  }
  return ret;
#else
  (void)publisher;
  (void)subscription_count;
  RMW_SET_ERROR_MSG(
    "Function not available; enable RMW_UXRCE_GRAPH configuration profile before using");
  return RMW_RET_UNSUPPORTED;
#endif  // RMW_UXRCE_GRAPH
}

rmw_ret_t
rmw_publisher_assert_liveliness(
  const rmw_publisher_t * publisher)
{
  (void)publisher;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_publisher_get_actual_qos(
  const rmw_publisher_t * publisher,
  rmw_qos_profile_t * qos)
{
  (void)qos;

  rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;
  qos = &custom_publisher->qos;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_borrow_loaned_message(
  const rmw_publisher_t * publisher,
  const rosidl_message_type_support_t * type_support,
  void ** ros_message)
{
  (void)publisher;
  (void)type_support;
  (void)ros_message;

  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_return_loaned_message_from_publisher(
  const rmw_publisher_t * publisher,
  void * loaned_message)
{
  (void)publisher;
  (void)loaned_message;

  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_destroy_publisher(
  rmw_node_t * node,
  rmw_publisher_t * publisher)
{
  rmw_ret_t result_ret = RMW_RET_OK;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier)) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!node->data) {
    RMW_SET_ERROR_MSG("node imp is null");
    result_ret = RMW_RET_ERROR;
  } else if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (!is_uxrce_rmw_identifier_valid(publisher->implementation_identifier)) {
    RMW_SET_ERROR_MSG("publisher handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!publisher->data) {
    RMW_SET_ERROR_MSG("publisher imp is null");
    result_ret = RMW_RET_ERROR;
  } else {
    rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;
    rmw_uxrce_node_t * custom_node = custom_publisher->owner_node;

    destroy_topic(custom_publisher->topic);

    uint16_t delete_writer = uxr_buffer_delete_entity(
      &custom_publisher->owner_node->context->session,
      *custom_publisher->owner_node->context->creation_destroy_stream,
      custom_publisher->datawriter_id);
    uint16_t delete_publisher = uxr_buffer_delete_entity(
      &custom_publisher->owner_node->context->session,
      *custom_publisher->owner_node->context->creation_destroy_stream,
      custom_publisher->publisher_id);

    bool ret = run_xrce_session(custom_node->context, delete_writer);
    ret &= run_xrce_session(custom_node->context, delete_publisher);
    if (!ret) {
      result_ret = RMW_RET_ERROR;
    }

    rmw_uxrce_fini_publisher_memory(publisher);
  }

  return result_ret;
}
