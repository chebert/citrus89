#if 0
#include "pool.h"

struct PoolFreeList {
  struct PoolFreeList *next_free;
};
typedef struct PoolFreeList *PoolFreeList;

struct Pool {
  Buffer buffer;
  PoolFreeList next_free;
};

static PoolFreeList PoolBufferElement(Pool pool, size_t element_index, size_t num_bytes_per_element) {
  return (PoolFreeList)&pool->buffer->buffer[element_index * num_bytes_per_element];
}

Pool MakePool(size_t num_elements, size_t num_bytes_per_element) {
  AllocateStruct(Pool, pool);
  pool->buffer = MakeBuffer(num_elements * num_bytes_per_element);
  pool->next_free = PoolBufferElement(pool, 0, num_bytes_per_element);
  /* Each element points to the next element.
   * The last element points to NULL */
  for (size_t element_index = 0; element_index < num_elements; ++element_index) {
    PoolFreeList this_element = PoolBufferElement(pool, element_index, num_bytes_per_element);
    PoolFreeList next_element = PoolBufferElement(pool, element_index+1, num_bytes_per_element);
    this_element->next_free = (element_index < num_elements-1)
      ? next_element
      : NULL;
  }
}

u1 *PoolAllocate(Pool pool) {
  /* Pop an element off of the free list */
  PoolFreeList result = pool->next_free;
  if (result) {
    pool->next_free = pool->next_free->next_free;
  }
  return (u1 *)result;
}
void PoolFree(Pool pool, Any element) {
  /* Add element back onto the free list */
  PoolFreeList e = (PoolFreeList)element;
  e->next_free = pool->next_free;
  pool->next_free = e;
}
#endif
