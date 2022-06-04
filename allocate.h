#ifndef ALLOCATE_H
#define ALLOCATE_H

#include <stdint.h>

void *Allocate(size_t num_bytes);

#define AllocateStruct(struct_name)\
  (struct_name)Allocate(sizeof(struct struct_name))

#endif
