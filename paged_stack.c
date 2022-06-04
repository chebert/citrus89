#include "paged_stack.h"

#include <stdlib.h>

#include "type.h"

#define STRUCT_Header(f)\
  f(Header, previous)\
  f(b4, is_page)
Struct(Header);
DefineStruct(PagedStack);

static Stack CurrentStack(PagedStack stack) {
  return &stack->stack_storage;
}
static Bytes PageData(Header header) {
  return (Bytes)header + sizeof(struct Header);
}

PagedStack InitPagedStack(PagePool pool, PagedStack stack) {
  stack = stack ? stack : AllocateStruct(PagedStack);
  stack->pool = pool;
  stack->latest_page = NULL;
  _ByteBuffer(NULL, 0, &stack->stack_buffer_storage);
  InitStack(&stack->stack_buffer_storage, &stack->stack_storage);
  return stack;
}

void *PagedStackAllocate(PagedStack stack, size_t num_bytes) {
  const size_t page_size = stack->pool->page_size_in_bytes;
  if (page_size / 2 < num_bytes) {
    // Large allocation; use malloc
    // Allocate the data + a header
    Header header = malloc(sizeof(struct Header)+num_bytes);
    if (!header) return NULL;
    header->is_page = FALSE;
    header->previous = stack->latest_page;
    stack->latest_page = header;
    return PageData(header);
  } else {
    // Try allocating directly from the current stack.
    void *result = StackAllocate(CurrentStack(stack), num_bytes);
    if (result) return result;

    // Otherwise allocate another page.
    Header page = (Header)PagePoolAllocate(stack->pool);
    if (!page) return NULL;

    page->previous = stack->latest_page;
    page->is_page = TRUE;
    stack->latest_page = page;
    InitStack(
        _ByteBuffer(PageData(page), page_size, &stack->stack_buffer_storage),
        &stack->stack_storage);
    return StackAllocate(CurrentStack(stack), num_bytes);
  }
}

void PagedStackClear(PagedStack stack) {
  Header current = stack->latest_page;
  while (current) {
    Header previous = current->previous;
    if (current->is_page) {
      PagePoolFree(stack->pool, (void *)current);
    } else {
      free((void *)current);
    }
    current = previous;
  }
  stack->latest_page = NULL;
  InitStack(_ByteBuffer(0, 0, &stack->stack_buffer_storage), &stack->stack_storage);
}
