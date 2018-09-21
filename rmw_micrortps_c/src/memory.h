#ifndef RMW_MICRORTPS_MEMORY_H_
#define RMW_MICRORTPS_MEMORY_H_

#include <stddef.h>
#include <stdbool.h>

struct Item
{
    struct Item* prev;
    struct Item* next;
    void* data;
};
struct MemPool
{
    struct Item* allocateditems;
    struct Item* freeitems;

    size_t size;
};

void link_next(struct Item* current, struct Item* next, void* data);
void link_prev(struct Item* previous, struct Item* current, void* data);
void set_mem_pool(struct MemPool* mem, struct Item* first);
bool has_memory(struct MemPool* mem);
struct Item* get_memory(struct MemPool* mem);
void put_memory(struct MemPool* mem, struct Item* item);
void free_mem_pool(struct MemPool* mem);

#endif // !RMW_MICRORTPS_MEMORY_H_