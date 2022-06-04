#ifndef POOL_H
#define POOL_H

#include "type.h"

typedef struct Pool *pool;

Pool MakePool(size_t num_elements, size_t num_bytes_per_element);

u1 *PoolAllocate(Pool pool);
void PoolFree(Pool pool, Any element);

#endif
