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

#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdbool.h>
#include <stddef.h>

struct Item
{
  struct Item * prev;
  struct Item * next;
  void * data;
};
struct rmw_uxrce_mempool_t
{
  struct Item * allocateditems;
  struct Item * freeitems;

  size_t size;
};

void link_next(struct Item * current, struct Item * next, void * data);
void link_prev(struct Item * previous, struct Item * current, void * data);
void set_mem_pool(struct rmw_uxrce_mempool_t * mem, struct Item * first);
bool has_memory(struct rmw_uxrce_mempool_t * mem);
struct Item * get_memory(struct rmw_uxrce_mempool_t * mem);
void put_memory(struct rmw_uxrce_mempool_t * mem, struct Item * item);
void free_mem_pool(struct rmw_uxrce_mempool_t * mem);

#endif  // MEMORY_H_
