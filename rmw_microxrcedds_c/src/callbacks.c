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

#include "./callbacks.h"

void on_status( struct uxrSession* session,
                uxrObjectId object_id,
                uint16_t request_id,
                uint8_t status,
                void* args)
{
  (void)session;
  (void)object_id;
  (void)request_id;
  (void)status;
  (void)args;
}

void on_topic(  struct uxrSession* session,
                uxrObjectId object_id,
                uint16_t request_id,
                uxrStreamId stream_id,
                struct ucdrBuffer* ub,
                uint16_t length,
                void* args)
{
  (void)session;
  (void)request_id;
  (void)stream_id;
  (void)args;

  struct rmw_uxrce_mempool_item_t * subscription_item = subscription_memory.allocateditems;
  while (subscription_item != NULL) {
    rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)subscription_item->data;
    if ((custom_subscription->datareader_id.id == object_id.id) &&
      (custom_subscription->datareader_id.type == object_id.type))
    {
      custom_subscription->micro_buffer_lenght[custom_subscription->history_write_index] = length;
      ucdr_deserialize_array_uint8_t(ub, custom_subscription->micro_buffer[custom_subscription->history_write_index], length);

      // TODO(pablogs9): Circular overlapping buffer implemented: use qos
      if (custom_subscription->micro_buffer_in_use && custom_subscription->history_write_index == custom_subscription->history_read_index){
        custom_subscription->history_read_index = (custom_subscription->history_read_index + 1) % RMW_UXRCE_MAX_HISTORY;
      }

      custom_subscription->history_write_index = (custom_subscription->history_write_index + 1) % RMW_UXRCE_MAX_HISTORY;
      custom_subscription->micro_buffer_in_use = true;

      break;
    }
    subscription_item = subscription_item->next;
  }
}

void on_request(struct uxrSession* session,
                uxrObjectId object_id,
                uint16_t request_id,
                SampleIdentity* sample_id,
                struct ucdrBuffer* ub,
                uint16_t length,
                void* args)
{
  (void)session;
  (void)object_id;
  (void)args;

  struct rmw_uxrce_mempool_item_t * service_item = service_memory.allocateditems;
  while (service_item != NULL) {
    rmw_uxrce_service_t * custom_service = (rmw_uxrce_service_t *)service_item->data;
    if (custom_service->request_id == request_id){
      custom_service->micro_buffer_lenght[custom_service->history_write_index] = length;
      ucdr_deserialize_array_uint8_t(ub, custom_service->micro_buffer[custom_service->history_write_index], length);
      memcpy(&custom_service->sample_id[custom_service->history_write_index],
          sample_id, sizeof(SampleIdentity));

      // TODO(pablogs9): Circular overlapping buffer implemented: use qos
      if (custom_service->micro_buffer_in_use && custom_service->history_write_index == custom_service->history_read_index){
        custom_service->history_read_index = (custom_service->history_read_index + 1) % RMW_UXRCE_MAX_HISTORY;
      }

      custom_service->history_write_index = (custom_service->history_write_index + 1) % RMW_UXRCE_MAX_HISTORY;
      custom_service->micro_buffer_in_use = true;

      break;
    }
    service_item = service_item->next;
  }
}

void on_reply(  struct uxrSession* session,
                uxrObjectId object_id,
                uint16_t request_id,
                uint16_t reply_id,
                struct ucdrBuffer* ub,
                uint16_t length,
                void* args)
{
  (void)session;
  (void)object_id;
  (void)args;

  struct rmw_uxrce_mempool_item_t * client_item = client_memory.allocateditems;
  while (client_item != NULL) {
    rmw_uxrce_client_t * custom_client = (rmw_uxrce_client_t *)client_item->data;
    if (custom_client->request_id == request_id)
    {
      custom_client->micro_buffer_lenght[custom_client->history_write_index] = length;
      ucdr_deserialize_array_uint8_t(ub, custom_client->micro_buffer[custom_client->history_write_index], length);
      custom_client->reply_id[custom_client->history_write_index] = reply_id;

      // TODO(pablogs9): Circular overlapping buffer implemented: use qos
      if (custom_client->micro_buffer_in_use && custom_client->history_write_index == custom_client->history_read_index){
        custom_client->history_read_index = (custom_client->history_read_index + 1) % RMW_UXRCE_MAX_HISTORY;
      }

      custom_client->history_write_index = (custom_client->history_write_index + 1) % RMW_UXRCE_MAX_HISTORY;
      custom_client->micro_buffer_in_use = true;

      break;
  }
  client_item = client_item->next;
  }
}