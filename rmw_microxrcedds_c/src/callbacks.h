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

#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include "./types.h"
#include <rmw_microxrcedds_c/config.h>

void on_status(uxrSession * session, uxrObjectId object_id, uint16_t request_id, uint8_t status, void * args);

void on_topic(uxrSession * session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer * serialization, void * args);

void on_request(uxrSession* session, uxrObjectId object_id, uint16_t request_id, SampleIdentity* sample_id, uint8_t* request_buffer, size_t request_len, void* args);

void on_reply(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uint16_t reply_id, uint8_t* buffer, size_t len, void* args);

#endif  // CALLBACKS_H_
