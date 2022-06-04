#ifndef PAGE_POOL_H
#define PAGE_POOL_H

#include "type.h"
#include "struct.h"


STRUCT_TYPEDEF(Page);
#define STRUCT_PagePool(f)\
  f(Page, next_free_page)\
  f(size_t, page_size_in_bytes)
DeclareStruct(PagePool);

PagePool InitPagePool(size_t page_size_in_bytes, PagePool pool_storage);

void *PagePoolAllocate(PagePool pool);
void PagePoolFree(PagePool pool, void *page);

#endif
