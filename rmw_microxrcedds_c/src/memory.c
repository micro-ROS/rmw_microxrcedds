#include "memory.h"

void link_next(struct Item* current, struct Item* next, void* data)
{
    if (current)
    {
        current->next = next;
        if (next)
        {
            next->prev = current;
        }
        current->data = data;
    }
}

void link_prev(struct Item* previous, struct Item* current, void* data)
{
    if (current)
    {
        current->prev = previous;
        if (previous)
        {
            previous->next = current;
        }
        current->data = data;
    }
}

void set_mem_pool(struct MemPool* mem, struct Item* first)
{
    mem->freeitems      = first;
    mem->allocateditems = NULL;
}

void free_mem_pool(struct MemPool* mem)
{
    if (mem->allocateditems)
    {
        struct Item* old_free_head = mem->freeitems;
        mem->freeitems             = mem->allocateditems;
        mem->freeitems->prev       = NULL;
        mem->allocateditems        = NULL;

        struct Item* free_item = mem->freeitems;
        while (free_item->next)
        {
            free_item = free_item->next;
        }
        free_item->next     = old_free_head;
        old_free_head->prev = free_item;
    }
}

bool has_memory(struct MemPool* mem)
{
    return mem->freeitems != NULL ? true : false;
}

struct Item* get_memory(struct MemPool* mem)
{
    struct Item* item = NULL;
    if (has_memory(mem))
    {
        // Gets item from free pool
        item           = mem->freeitems;
        mem->freeitems = item->next;
        if (mem->freeitems)
        {
            mem->freeitems->prev = NULL;
        }

        // Puts item in allocated pool
        item->next = mem->allocateditems;
        if (item->next)
        {
            item->next->prev = item;
        }
        item->prev          = NULL;
        mem->allocateditems = item;
    }
    return item;
}

void put_memory(struct MemPool* mem, struct Item* item)
{
    // Gets item from allocated pool
    if (item->prev)
    {
        item->prev->next = item->next;
    }
    if (item->next)
    {
        item->next->prev = item->prev;
    }

    if (mem->allocateditems == item)
    {
        mem->allocateditems = item->next;
    }

    // if (mem->freeitems)
    // {
    //     mem->freeitems->prev    = NULL;
    // }

    // Puts item in free pool
    item->next = mem->freeitems;
    if (item->next)
    {
        item->next->prev = item;
    }
    item->prev     = NULL;
    mem->freeitems = item;
}