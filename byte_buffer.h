#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include "struct.h"
#include "type.h"

#define STRUCT_ByteBuffer(f)\
  f(Bytes, buffer)\
  f(size_t, size)
DeclareStruct(ByteBuffer);

#endif
