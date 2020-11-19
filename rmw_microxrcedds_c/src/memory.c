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

#include "./memory.h"  // NOLINT


void link_next(
  rmw_uxrce_mempool_item_t * current, rmw_uxrce_mempool_item_t * next,
  void * data)
{
  if (current) {
    current->next = next;
    if (next) {
      next->prev = current;
    }
    current->data = data;
  }
}

void link_prev(
  rmw_uxrce_mempool_item_t * previous,
  rmw_uxrce_mempool_item_t * current, void * data)
{
  if (current) {
    current->prev = previous;
    if (previous) {
      previous->next = current;
    }
    current->data = data;
  }
}

void set_mem_pool(rmw_uxrce_mempool_t * mem, rmw_uxrce_mempool_item_t * first)
{
  mem->freeitems = first;
  mem->allocateditems = NULL;
}

void free_mem_pool(rmw_uxrce_mempool_t * mem)
{
  if (mem->allocateditems) {
    rmw_uxrce_mempool_item_t * old_free_head = mem->freeitems;
    mem->freeitems = mem->allocateditems;
    mem->freeitems->prev = NULL;
    mem->allocateditems = NULL;

    rmw_uxrce_mempool_item_t * free_item = mem->freeitems;
    while (free_item->next) {
      free_item = free_item->next;
    }
    free_item->next = old_free_head;
    old_free_head->prev = free_item;
  }
}

bool has_memory(rmw_uxrce_mempool_t * mem)
{
  return mem->freeitems != NULL ? true : false;
}

rmw_uxrce_mempool_item_t * get_memory(rmw_uxrce_mempool_t * mem)
{
  rmw_uxrce_mempool_item_t * item = NULL;
  if (has_memory(mem)) {
    // Gets item from free pool
    item = mem->freeitems;
    mem->freeitems = item->next;
    if (mem->freeitems) {
      mem->freeitems->prev = NULL;
    }

    // Puts item in allocated pool
    item->next = mem->allocateditems;
    if (item->next) {
      item->next->prev = item;
    }
    item->prev = NULL;
    mem->allocateditems = item;
  }
  return item;
}

void put_memory(rmw_uxrce_mempool_t * mem, rmw_uxrce_mempool_item_t * item)
{
  // Gets item from allocated pool
  if (item->prev) {
    item->prev->next = item->next;
  }
  if (item->next) {
    item->next->prev = item->prev;
  }

  if (mem->allocateditems == item) {
    mem->allocateditems = item->next;
  }

  // if (mem->freeitems){
  //     mem->freeitems->prev = NULL;
  // }

  // Puts item in free pool
  item->next = mem->freeitems;
  if (item->next) {
    item->next->prev = item;
  }
  item->prev = NULL;
  mem->freeitems = item;
}
