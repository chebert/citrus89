#include "stack.h"

#include "allocator.h"

DefineStruct(Stack);

Stack MakeStack(ByteBuffer buffer) {
  return Stack_(buffer, 0);
}
Stack InitStack(ByteBuffer buffer, Stack stack) {
  return _Stack(buffer, 0, stack);
}

u1 *StackAllocate(Stack stack, size_t num_bytes) {
  if (stack->buffer->size - stack->index < num_bytes) {
    return NULL;
  } else {
    u1 *result = &stack->buffer->buffer[stack->index];
    stack->index += num_bytes;
    return result;
  }
}
u1 *StackClear(Stack stack) {
  stack->index = 0;
}
