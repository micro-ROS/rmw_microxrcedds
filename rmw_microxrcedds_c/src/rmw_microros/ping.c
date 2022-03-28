// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#include <rmw_microxrcedds_c/config.h>
#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/ret_types.h>

#include <uxr/client/client.h>
#include <uxr/client/util/ping.h>

#include "../rmw_microros_internal/rmw_uxrce_transports.h"
#include "../rmw_microros_internal/types.h"
#include "./rmw_microros_internal/utils.h"

extern rmw_uxrce_transport_params_t rmw_uxrce_transport_default_params;

rmw_ret_t rmw_uros_ping_agent(
  const int timeout_ms,
  const uint8_t attempts)
{
  bool success = false;

  if (!session_memory.is_initialized || NULL == session_memory.allocateditems) {
    // There is no session available to ping. Init transport is required.
#ifdef RMW_UXRCE_TRANSPORT_SERIAL
    uxrSerialTransport transport;
#elif defined(RMW_UXRCE_TRANSPORT_UDP)
    uxrUDPTransport transport;
#elif defined(RMW_UXRCE_TRANSPORT_TCP)
    uxrTCPTransport transport;
#elif defined(RMW_UXRCE_TRANSPORT_CUSTOM)
    uxrCustomTransport transport;
    transport.framing = rmw_uxrce_transport_default_params.framing;
    transport.args = rmw_uxrce_transport_default_params.args;
    transport.open = rmw_uxrce_transport_default_params.open_cb;
    transport.close = rmw_uxrce_transport_default_params.close_cb;
    transport.write = rmw_uxrce_transport_default_params.write_cb;
    transport.read = rmw_uxrce_transport_default_params.read_cb;
#endif /* ifdef RMW_UXRCE_TRANSPORT_SERIAL */
    rmw_ret_t ret = rmw_uxrce_transport_init(NULL, NULL, (void *)&transport);

    if (RMW_RET_OK != ret) {
      return ret;
    }

    success = uxr_ping_agent_attempts(&transport.comm, timeout_ms, attempts);
    CLOSE_TRANSPORT(&transport);
  } else {
    // There is a session available to ping. Using session.
    rmw_uxrce_mempool_item_t * item = session_memory.allocateditems;
    do {
      rmw_context_impl_t * context = (rmw_context_impl_t *)item->data;

      success = uxr_ping_agent_session(&context->session, timeout_ms, attempts);

      item = item->next;
    } while (NULL != item && !success);
  }

  return success ? RMW_RET_OK : RMW_RET_ERROR;
}

rmw_ret_t rmw_uros_ping_agent_options(
  const int timeout_ms,
  const uint8_t attempts,
  rmw_init_options_t * rmw_options)
{
  bool success = false;

#ifdef RMW_UXRCE_TRANSPORT_SERIAL
  uxrSerialTransport transport;
#elif defined(RMW_UXRCE_TRANSPORT_UDP)
  uxrUDPTransport transport;
#elif defined(RMW_UXRCE_TRANSPORT_TCP)
  uxrTCPTransport transport;
#elif defined(RMW_UXRCE_TRANSPORT_CUSTOM)
  uxrCustomTransport transport;
  transport.framing = rmw_options->impl->transport_params.framing;
  transport.args = rmw_options->impl->transport_params.args;
  transport.open = rmw_options->impl->transport_params.open_cb;
  transport.close = rmw_options->impl->transport_params.close_cb;
  transport.write = rmw_options->impl->transport_params.write_cb;
  transport.read = rmw_options->impl->transport_params.read_cb;
#endif /* ifdef RMW_UXRCE_TRANSPORT_SERIAL */
  rmw_ret_t ret = rmw_uxrce_transport_init(NULL, rmw_options->impl, (void *)&transport);

  if (RMW_RET_OK != ret) {
    return ret;
  }

  success = uxr_ping_agent_attempts(&transport.comm, timeout_ms, attempts);
  CLOSE_TRANSPORT(&transport);

  return success ? RMW_RET_OK : RMW_RET_ERROR;
}

rmw_ret_t rmw_uros_regenerate_entities()
{
  bool success = true;

  if (!session_memory.is_initialized || NULL == session_memory.allocateditems) {
    return RMW_RET_ERROR;
  }

  rmw_uxrce_mempool_item_t * item = session_memory.allocateditems;
  rmw_context_impl_t * context = (rmw_context_impl_t *)item->data;

  bool ping_success = uxr_ping_agent_attempts(context->session.comm, 1000, 1);

  if (!ping_success) {
    return RMW_RET_ERROR;
  }

  // Regenerate sessions
  {
    rmw_uxrce_mempool_item_t * item = session_memory.allocateditems;
    while (NULL != item) {
      rmw_context_impl_t * context = (rmw_context_impl_t *)item->data;

      uxr_create_session(&context->session);

      item = item->next;
    }
  }

  // Regenerate nodes
  {
    rmw_uxrce_mempool_item_t * item = node_memory.allocateditems;
    while (NULL != item) {
      rmw_uxrce_node_t * custom_node = (rmw_uxrce_node_t *)item->data;
      uint16_t req = UXR_INVALID_REQUEST_ID;

      if (strcmp(custom_node->node_namespace, "/") == 0) {
        snprintf(node_name_buffer, sizeof(node_name_buffer), "%s", custom_node->node_name);
      } else {
        snprintf(
          node_name_buffer, sizeof(node_name_buffer), "%s/%s", custom_node->node_namespace,
          custom_node->node_name);
      }

      req = uxr_buffer_create_participant_bin(
        &custom_node->context->session,
        *custom_node->context->creation_stream,
        custom_node->participant_id,
        custom_node->domain_id,
        node_name_buffer,
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_node->context, custom_node->context->creation_stream, req,
        custom_node->context->creation_timeout);

      item = item->next;
    }
  }

  // Regenerate publishers
  {
    rmw_uxrce_mempool_item_t * item = publisher_memory.allocateditems;
    while (NULL != item) {
      rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)item->data;
      uint16_t req = UXR_INVALID_REQUEST_ID;

      generate_topic_name(
        custom_publishers->topic.topic_name, topic_buffer_1,
        sizeof(topic_buffer_1));
      generate_type_name(
        custom_publisher->topic.type_support_callbacks.msg, type_buffer_1,
        sizeof(type_buffer_1));

      req = uxr_buffer_create_topic_bin(
        &custom_publisher->owner_node->context->session,
        *custom_publisher->owner_node->context->creation_stream,
        custom_publisher->topic.topic_id,
        custom_publisher->owner_node->participant_id,
        topic_buffer_1,
        type_buffer_1,
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_publisher->owner_node->context,
        custom_publisher->owner_node->context->creation_stream, req,
        custom_publisher->owner_node->context->creation_timeout);

      req = uxr_buffer_create_publisher_bin(
        &custom_publisher->owner_node->context->session,
        *custom_publisher->owner_node->context->creation_stream,
        custom_publisher->publisher_id,
        custom_publisher->owner_node->participant_id,
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_publisher->owner_node->context,
        custom_publisher->owner_node->context->creation_stream, req,
        custom_publisher->owner_node->context->creation_timeout);

      req = uxr_buffer_create_datawriter_bin(
        &custom_publisher->owner_node->context->session,
        *custom_publisher->owner_node->context->creation_stream,
        custom_publisher->datawriter_id,
        custom_publisher->publisher_id,
        custom_publisher->topic.topic_id,
        convert_qos_profile(&custom_publisher->qos),
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_publisher->owner_node->context,
        custom_publisher->owner_node->context->creation_stream, req,
        custom_publisher->owner_node->context->creation_timeout);

      item = item->next;
    }
  }

  // Regenerate subscribers
  {
    rmw_uxrce_mempool_item_t * item = subscription_memory.allocateditems;
    while (NULL != item) {
      rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)item->data;
      uint16_t req = UXR_INVALID_REQUEST_ID;

      generate_topic_name(
        custom_subscription->topic.topic_name, topic_buffer_1,
        sizeof(topic_buffer_1));
      generate_type_name(
        custom_subscription->topic.type_support_callbacks.msg, type_buffer_1,
        sizeof(type_buffer_1));

      req = uxr_buffer_create_topic_bin(
        &custom_subscription->owner_node->context->session,
        *custom_subscription->owner_node->context->creation_stream,
        custom_subscription->topic.topic_id,
        custom_subscription->owner_node->participant_id,
        topic_buffer_1,
        type_buffer_1,
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_subscription->owner_node->context,
        custom_subscription->owner_node->context->creation_stream, req,
        custom_subscription->owner_node->context->creation_timeout);

      req = uxr_buffer_create_subscriber_bin(
        &custom_subscription->owner_node->context->session,
        *custom_subscription->owner_node->context->creation_stream,
        custom_subscription->subscriber_id,
        custom_subscription->owner_node->participant_id,
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_subscription->owner_node->context,
        custom_subscription->owner_node->context->creation_stream, req,
        custom_subscription->owner_node->context->creation_timeout);

      req = uxr_buffer_create_datareader_bin(
        &custom_subscription->owner_node->context->session,
        *custom_subscription->owner_node->context->creation_stream,
        custom_subscription->datareader_id,
        custom_subscription->subscriber_id,
        custom_subscription->topic.topic_id,
        convert_qos_profile(&custom_subscription->qos),
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_subscription->owner_node->context,
        custom_subscription->owner_node->context->creation_stream, req,
        custom_subscription->owner_node->context->creation_timeout);

      uxrDeliveryControl delivery_control;
      delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
      delivery_control.min_pace_period = 0;
      delivery_control.max_elapsed_time = UXR_MAX_ELAPSED_TIME_UNLIMITED;
      delivery_control.max_bytes_per_second = UXR_MAX_BYTES_PER_SECOND_UNLIMITED;

      uxrStreamId data_request_stream_id =
        (custom_subscription->qos.reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
        custom_subscription->owner_node->context->best_effort_input :
        custom_subscription->owner_node->context->reliable_input;

      uxr_buffer_request_data(
        &custom_subscription->owner_node->context->session,
        *custom_subscription->owner_node->context->creation_stream,
        custom_subscription->datareader_id,
        data_request_stream_id, &delivery_control);

      item = item->next;
    }
  }

  // Regenerate requesters
  {
    rmw_uxrce_mempool_item_t * item = service_memory.allocateditems;
    while (NULL != item) {
      rmw_uxrce_service_t * custom_service = (rmw_uxrce_service_t *)item->data;
      uint16_t req = UXR_INVALID_REQUEST_ID;

      generate_service_types(
        custom_service->topic.type_support_callbacks.srv, type_buffer_1, type_buffer_2,
        RMW_UXRCE_TYPE_NAME_MAX_LENGTH);

      generate_service_topics(
        custom_service->topic.topic_name, topic_buffer_1, topic_buffer_2,
        RMW_UXRCE_TOPIC_NAME_MAX_LENGTH);

      req = uxr_buffer_create_replier_bin(
        &custom_service->owner_node->context->session,
        *custom_service->owner_node->context->creation_stream,
        custom_service->service_id,
        custom_service->owner_node->participant_id,
        (char *) custom_service->topic.topic_name,
        type_buffer_1,
        type_buffer_2,
        topic_buffer_1,
        topic_buffer_2,
        convert_qos_profile(&custom_service->qos),
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_service->owner_node->context,
        custom_service->owner_node->context->creation_stream, req,
        custom_service->owner_node->context->creation_timeout);

      uxrStreamId data_request_stream_id =
        (custom_service->qos.reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
        custom_service->owner_node->context->best_effort_input :
        custom_service->owner_node->context->reliable_input;

      uxrDeliveryControl delivery_control;
      delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
      delivery_control.min_pace_period = 0;
      delivery_control.max_elapsed_time = UXR_MAX_ELAPSED_TIME_UNLIMITED;
      delivery_control.max_bytes_per_second = UXR_MAX_BYTES_PER_SECOND_UNLIMITED;

      custom_service->service_data_resquest = uxr_buffer_request_data(
        &custom_service->owner_node->context->session,
        *custom_service->owner_node->context->creation_stream, custom_service->service_id,
        data_request_stream_id, &delivery_control);
      item = item->next;
    }
  }

  // Regenerate repliers
  {
    rmw_uxrce_mempool_item_t * item = client_memory.allocateditems;
    while (NULL != item) {
      rmw_uxrce_client_t * custom_client = (rmw_uxrce_client_t *)item->data;
      uint16_t req = UXR_INVALID_REQUEST_ID;

      generate_service_types(
        custom_client->topic.type_support_callbacks.srv, type_buffer_1, type_buffer_2,
        RMW_UXRCE_TYPE_NAME_MAX_LENGTH);
      generate_service_topics(
        custom_client->topic.topic_name, topic_buffer_1, topic_buffer_2,
        RMW_UXRCE_TOPIC_NAME_MAX_LENGTH);

      req = uxr_buffer_create_requester_bin(
        &custom_client->owner_node->context->session,
        *custom_client->owner_node->context->creation_stream,
        custom_client->client_id,
        custom_client->owner_node->participant_id,
        (char *) custom_client->topic.topic_name,
        type_buffer_1,
        type_buffer_2,
        topic_buffer_1,
        topic_buffer_2,
        convert_qos_profile(&custom_client->qos),
        UXR_REPLACE | UXR_REUSE);

      run_xrce_session(
        custom_client->owner_node->context,
        custom_client->owner_node->context->creation_stream, req,
        custom_client->owner_node->context->creation_timeout);

      uxrStreamId data_request_stream_id =
        (custom_client->qos.reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
        custom_client->owner_node->context->best_effort_input :
        custom_client->owner_node->context->reliable_input;

      uxrDeliveryControl delivery_control;
      delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
      delivery_control.min_pace_period = 0;
      delivery_control.max_elapsed_time = UXR_MAX_ELAPSED_TIME_UNLIMITED;
      delivery_control.max_bytes_per_second = UXR_MAX_BYTES_PER_SECOND_UNLIMITED;

      custom_client->client_data_request = uxr_buffer_request_data(
        &custom_client->owner_node->context->session,
        *custom_client->owner_node->context->creation_stream, custom_client->client_id,
        data_request_stream_id, &delivery_control);

      item = item->next;
    }
  }

  return success ? RMW_RET_OK : RMW_RET_ERROR;
}
