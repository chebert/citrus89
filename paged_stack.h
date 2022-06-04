#ifndef PAGED_STACK_H
#define PAGED_STACK_H

#include "page_pool.h"
#include "stack.h"
#include "struct.h"

typedef struct Header *Header;
#define STRUCT_PagedStack(f)\
  f(PagePool, pool)\
  f(Header, latest_page)\
  f(struct ByteBuffer, stack_buffer_storage)\
  f(struct Stack, stack_storage)
DeclareStruct(PagedStack);

PagedStack InitPagedStack(PagePool pool, PagedStack storage);

void *PagedStackAllocate(PagedStack stack, size_t num_bytes);
void PagedStackClear(PagedStack stack);

#endif
