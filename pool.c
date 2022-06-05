#include "pool.h"

#define STRUCT_PoolFreeList(f)\
  f(PoolFreeList, next)
Struct(PoolFreeList);

DefineStruct(Pool);

Pool MakePool() { return InitPool(NULL); }
Pool InitPool(Pool pool) {
  pool = pool ? pool : AllocateStruct(Pool);
  pool->next = NULL;
  return pool;
}

void *PoolAllocate(Pool pool) {
  PoolFreeList result = pool->next;
  if (result) {
    pool->next = result->next;
  }
  return (void *)result;
}

void PoolFree(Pool pool, void *element) {
  PoolFreeList next = (PoolFreeList)element;
  next->next = pool->next;
  pool->next = next;
}
