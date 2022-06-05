#include "page_pool.h"

#include <stdlib.h>

#include "allocate.h"

DefineStruct(PagePool);

PagePool MakePagePool(size_t page_size_in_bytes) {
  return InitPagePool(page_size_in_bytes, NULL, NULL);
}
PagePool InitPagePool(size_t page_size_in_bytes, Pool allocated_pages, PagePool page_pool) {
  return _PagePool(InitPool(allocated_pages), page_size_in_bytes, page_pool);
}

void *PagePoolAllocate(PagePool pool) {
  // Return a page from the allocated_pages pool, or malloc a new page.
  void *result;
  return
    (result = PoolAllocate(pool->allocated_pages))
    ? result
    : malloc(pool->page_size_in_bytes);
}
void PagePoolFree(PagePool pool, void *allocated_page) {
  // Add the allocated_page to the pool.
  PoolFree(pool->allocated_pages, allocated_page);
}
