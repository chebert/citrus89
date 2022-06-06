#include "paged_stack.h"

#include <stdlib.h>

#include "type.h"

#define STRUCT_PagedAllocationHeader(f)\
  f(PagedAllocationHeader, previous)\
  f(b4, is_page)
LocalStruct(PagedAllocationHeader)
DefineStruct(PagedStack)

static Bytes PagedAllocationData(PagedAllocationHeader header) {
  /* Return the data bytes associated with the header. */
  return (Bytes)header + Sizeof(PagedAllocationHeader);
}

PagedStack MakePagedStack(PagePool pool) {
  return InitPagedStack(pool, NULL, NULL, NULL);
}
PagedStack InitPagedStack(PagePool pool, ByteBuffer stack_byte_buffer, Stack stack, PagedStack paged_stack) {
  return _PagedStack(pool, NULL, InitStack(_ByteBuffer(NULL, 0, stack_byte_buffer), stack), paged_stack);
}

static PagedAllocationHeader PagedStackAddHeader(PagedStack stack, b4 is_page, void *header_bytes) {
  PagedAllocationHeader header = (PagedAllocationHeader)header_bytes;
  /* Given the header+data bytes, add a new header to the paged stack's allocation list. */
  if (!header_bytes) return NULL;
  header->is_page = is_page;
  /* Add the header to the list of allocations, so it can be freed later */
  header->previous = stack->last_allocation;
  stack->last_allocation = header;
  return header;
}

void *PagedStackAllocate(PagedStack stack, size_t num_bytes) {
  const size_t page_size = stack->pool->page_size_in_bytes;
  const size_t stack_size = page_size - Sizeof(PagedAllocationHeader);
  if (num_bytes > stack_size) {
    /* If the allocation is too big to allocate on a page, use malloc */
    PagedAllocationHeader header =
      PagedStackAddHeader(stack, FALSE, malloc(Sizeof(PagedAllocationHeader)+num_bytes));
    if (!header) return NULL;
    return PagedAllocationData(header);
  } else {
    /* Try allocating directly from the current stack. */
    void *result = StackAllocate(stack->stack, num_bytes);
    if (result) return result;
    else {
      /* Not enough memory on the current page: allocate another page. */
      PagedAllocationHeader header =
        PagedStackAddHeader(stack, TRUE, PagePoolAllocate(stack->pool));
      if (!header) return NULL;
      /* Set up the stack to point at the new page */
      _ByteBuffer(PagedAllocationData(header), stack_size, stack->stack->buffer);
      StackClear(stack->stack);

      /* Allocate from the stack */
      return StackAllocate(stack->stack, num_bytes);
    }
  }
}

void PagedStackClear(PagedStack stack) {
  /* Free all allocations on the allocation list. */

  /* For each header in the stack->last_allocation list. */
  PagedAllocationHeader header = stack->last_allocation;
  while (header) {
    PagedAllocationHeader previous_header = header->previous;

    /* Free the data */
    void *data = (void *)header;
    if (header->is_page) PagePoolFree(stack->pool, data);
    else free(data);

    header = previous_header;
  }
  stack->last_allocation = NULL;

  /* Reset the stack to an empty buffer. */
  _ByteBuffer(NULL, 0, stack->stack->buffer);
  StackClear(stack->stack);
}
