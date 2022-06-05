#include "global_allocator.h"

#include <stdlib.h>

jmp_buf global_allocator_jump_buffer;

static void *MallocAllocate(void *data, size_t num_bytes) {
  return malloc(num_bytes);
}

static Allocator GlobalAllocator(Allocator set_allocator) {
  static Allocator allocator = 0;
  if (set_allocator) allocator = set_allocator;
  else if (!allocator) allocator = MallocAllocator();
  return allocator;
}
static Allocator GetGlobalAllocator() { return GlobalAllocator(NULL); }

Allocator MallocAllocator() {
  static struct Allocator_Interface malloc_allocator_interface = { MallocAllocate };
  static struct Allocator static_malloc_allocator = { &malloc_allocator_interface, NULL, };
  return &static_malloc_allocator;
}

void SetGlobalAllocator(Allocator allocator) { GlobalAllocator(allocator); }
void *Allocate(size_t num_bytes) {
  void *result = AllocatorAllocate(GetGlobalAllocator(), num_bytes);
  if (!result)
    longjmp(global_allocator_jump_buffer, -1);
  return result;
}
