#ifndef PAGE_POOL_H
#define PAGE_POOL_H

#include "pool.h"
#include "type.h"
#include "struct.h"

#define STRUCT_PagePool(f)\
  f(Pool, allocated_pages)\
  f(size_t, page_size_in_bytes)
DeclareStruct(PagePool)

PagePool MakePagePool(size_t page_size_in_bytes);
PagePool InitPagePool(size_t page_size_in_bytes, Pool allocated_pages, PagePool page_pool);

void *PagePoolAllocate(PagePool pool);
void PagePoolFree(PagePool pool, void *page);

#endif
