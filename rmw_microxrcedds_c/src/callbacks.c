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

void on_status(
  uxrSession * session, uxrObjectId object_id, uint16_t request_id, uint8_t status,
  void * args)
{
  (void)session;
  (void)object_id;
  (void)request_id;
  (void)status;
  (void)args;
}

void on_topic(
  uxrSession * session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id,
  struct ucdrBuffer * serialization, void * args)
{
  (void)session;
  (void)request_id;
  (void)stream_id;

  struct Item * subscription_item = subscription_memory.allocateditems;
  while (subscription_item != NULL) {
    CustomSubscription * custom_subscription = (CustomSubscription *)subscription_item->data;
    if ((custom_subscription->datareader_id.id == object_id.id) &&
      (custom_subscription->datareader_id.type == object_id.type))
    { 
      ptrdiff_t lenght = serialization->final - serialization->iterator;
      
      custom_subscription->micro_buffer_lenght[custom_subscription->history_write_index] = lenght;
      memcpy(custom_subscription->micro_buffer[custom_subscription->history_write_index],
          serialization->iterator, lenght);
      

      // TODO (Pablo): Circular overlapping buffer implemented: use qos
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

void on_request(uxrSession* session, uxrObjectId object_id, uint16_t request_id, SampleIdentity* sample_id, uint8_t* request_buffer, size_t request_len, void* args)
{
  (void)session;
  (void)object_id;

  struct Item * service_item = service_memory.allocateditems;
  while (service_item != NULL) {
    CustomService * custom_service = (CustomService *)service_item->data;
    if (custom_service->request_id == request_id){
      custom_service->micro_buffer_lenght[custom_service->history_write_index] = request_len;
      memcpy(custom_service->micro_buffer[custom_service->history_write_index], 
          request_buffer, request_len);
      memcpy(&custom_service->sample_id[custom_service->history_write_index], 
          sample_id, sizeof(SampleIdentity));

      // TODO (Pablo): Circular overlapping buffer implemented: use qos
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

void on_reply(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uint16_t reply_id, uint8_t* buffer, size_t len, void* args)
{ 
  (void)session;
  (void)object_id;

  struct Item * client_item = client_memory.allocateditems;
  while (client_item != NULL) {
    CustomClient * custom_client = (CustomClient *)client_item->data;
    if (custom_client->request_id == request_id)
    { 

      custom_client->micro_buffer_lenght[custom_client->history_write_index] = len;
      memcpy(custom_client->micro_buffer[custom_client->history_write_index], buffer,len);
      custom_client->reply_id[custom_client->history_write_index] = reply_id;

      // TODO (Pablo): Circular overlapping buffer implemented: use qos
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