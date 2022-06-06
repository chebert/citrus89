#ifndef STACK_H
#define STACK_H

#include "byte_buffer.h"
#include "struct.h"
#include "type.h"


#define STRUCT_Stack(f)\
  f(ByteBuffer, buffer)\
  f(size_t, index)
DeclareStruct(Stack)

Stack MakeStack(ByteBuffer buffer);
Stack InitStack(ByteBuffer buffer, Stack stack);

u1 *StackAllocate(Stack stack, size_t num_bytes);
u1 *StackClear(Stack stack);

#endif
