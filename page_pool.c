#include "page_pool.h"

#include <stdlib.h>

#include "allocate.h"

struct Page {
  Page next_free_page;
};
DefineStruct(PagePool);

PagePool InitPagePool(size_t page_size_in_bytes, PagePool pool_storage) {
  return _PagePool(NULL, page_size_in_bytes, pool_storage);
}

void *PagePoolAllocate(PagePool pool) {
  if (pool->next_free_page) {
    Page page = pool->next_free_page;
    pool->next_free_page = page->next_free_page;
    return page;
  } else {
    return malloc(pool->page_size_in_bytes);
  }
}
void PagePoolFree(PagePool pool, void *allocated_page) {
  Page page = (Page)allocated_page;
  page->next_free_page = pool->next_free_page;
  pool->next_free_page = page;
}
