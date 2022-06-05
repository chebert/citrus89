#ifndef POOL_H
#define POOL_H

#include "struct.h"
#include "type.h"

STRUCT_TYPEDEF(PoolFreeList);
#define STRUCT_Pool(f)\
  f(PoolFreeList, next)
DeclareStruct(Pool);

Pool MakePool();
Pool InitPool(Pool pool);

void *PoolAllocate(Pool pool);
void PoolFree(Pool pool, void *element);

#endif
