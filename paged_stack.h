#ifndef PAGED_STACK_H
#define PAGED_STACK_H

#include "page_pool.h"
#include "stack.h"
#include "struct.h"

StructTypedef(PagedAllocationHeader)
#define STRUCT_PagedStack(f)\
  f(PagePool, pool)\
  f(PagedAllocationHeader, last_allocation)\
  f(Stack, stack)
DeclareStruct(PagedStack)

PagedStack MakePagedStack(PagePool pool);
PagedStack InitPagedStack(PagePool pool, ByteBuffer stack_byte_buffer, Stack stack, PagedStack paged_stack);

void *PagedStackAllocate(PagedStack stack, size_t num_bytes);
void PagedStackClear(PagedStack stack);

/* Provides stack-based allocation for small allocations (fits in a page of memory)
Uses a malloc-backed PagePool to allocate and free pages
For large allocations, uses malloc

If there are pages available, small allocations are O(1)
Freeing each page is O(1), while freeing each large allocation uses free()

Pre-allocating pages into the stack's page pool can avoid allocation costs of malloc at critical moments.
Multiple PagedStacks can share a single page pool. */

#endif
