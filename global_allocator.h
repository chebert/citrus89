#ifndef GLOBAL_ALLOCATOR_H
#define GLOBAL_ALLOCATOR_H

#include <setjmp.h>

#include "allocator.h"

extern jmp_buf global_allocator_jump_buffer;
#define CatchGlobalAllocatorOutOfMemoryError\
  if (setjmp(global_allocator_jump_buffer))

void SetGlobalAllocator(Allocator allocator);
Allocator MallocAllocator();

#endif
