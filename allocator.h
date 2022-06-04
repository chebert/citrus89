#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "allocate.h"
#include "interface.h"

#define INTERFACE_Allocator(M) M(Allocator, Allocate)
#define METHOD_Allocator_Allocate(R, A) R(void *) A(size_t, num_bytes)
DeclareInterface(Allocator);

#endif
