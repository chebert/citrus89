#include "interface.h"

#include <stdio.h>

#include "global_allocator.h"
#include "struct.h"
#include "page_size.h"
#include "paged_stack.h"
#include "tagged_union.h"

#define INTERFACE_Shape(m)\
  m(Shape, Area)\
  m(Shape, SetPosition)
#define METHOD_Shape_Area(r, a) r(f8)
#define METHOD_Shape_SetPosition(r, a) r(void) a(f8, x) a(f8, y)
Interface(Shape)

#define INTERFACE_Writer(m) m(Writer, Write)
#define METHOD_Writer_Write(r, a) r(FILE *) a(FILE *, stream)
Interface(Writer)

#define STRUCT_Circle(f)\
  f(f8, x)\
  f(f8, y)\
  f(f8, radius)
Struct(Circle)

Method(Circle, Shape, Area)
  return self->radius * self->radius * 3.14159;
EndMethod
Method(Circle, Shape, SetPosition)
  self->x = x;
  self->y = y;
EndMethod

Method(Circle, Writer, Write)
  fprintf(stream, "Circle { .x=%lf, .y=%lf, .radius=%lf }", self->x, self->y, self->radius);
  return stream;
EndMethod

/* A composite interface is just a struct with interfaces for fields */
#define STRUCT_WriterShape(f)\
  f(Writer, writer)\
  f(Shape, shape)
Struct(WriterShape)

#define OBJECT Circle
DefineConstructor(Shape)
DefineConstructor(Writer)
DefineCompositeConstructor(WriterShape)
#undef OBJECT

/* TODO: move these allocators */
Method(PagedStack, Allocator, Allocate)
  return PagedStackAllocate(self, num_bytes);
EndMethod

#define OBJECT PagedStack
DefineConstructor(Allocator)
#undef OBJECT

Method(Stack, Allocator, Allocate)
  return StackAllocate(self, num_bytes);
EndMethod

#define OBJECT Stack
DefineConstructor(Allocator)
#undef OBJECT

#define STATIC_MEMORY_SIZE_IN_BYTES 4096
static Allocator StaticAllocator() {
  static Byte memory[STATIC_MEMORY_SIZE_IN_BYTES];
  static struct ByteBuffer buffer;
  static struct Stack stack;
  static struct Allocator _allocator;
  static Allocator allocator = NULL;

  if (!allocator) {
    allocator = _StackAllocator(
        InitStack(_ByteBuffer(memory, sizeof(memory), &buffer), &stack),
        &_allocator);
  }
  return allocator;
}

/*
#define QUOTE(...) #__VA_ARGS__
#define PRINT(macro_call) printf("\"%s\" expands to \"%s\"\n", #macro_call, QUOTE(macro_call))
*/

/* NOTE:
Init functions (sometimes prefixed with _) take pointers to memory storage
Make functions (sometimes postfixed with _) allocate memory storage using Allocate
*/

#define TAGGED_UNION_Example(f)\
  f(Example, int, Index)\
  f(Example, Byte, Flag)\
  f(Example, f8, Distance)

TaggedUnion(Example)

int main(int argc, char **argv) {
  PagedStack stack;
  Circle circle;
  WriterShape ss;
  Example tagged;

  CatchGlobalAllocatorOutOfMemoryError {
    /* Run this code if we ever run out of memory. */
    fprintf(stderr, "Out of memory error\n");
    return -1;
  }
  /* Use a static stack to allocate */
  SetGlobalAllocator(StaticAllocator());

  stack = MakePagedStack(MakePagePool(GetPageSizeInBytesFromOS()));
  SetGlobalAllocator(PagedStackAllocator(stack));

  tagged = ExampleIndex_(32);
#define Case(field) TaggedCase(tagged, Example, field)
  TaggedSwitch(tagged) {
    Case(Index)    printf("index %d\n", Index);        EndCase;
    Case(Flag)     printf("flag %d\n", Flag);          EndCase;
    Case(Distance) printf("distance %lf\n", Distance); EndCase;
    default: printf("INVALID_TAG_TYPE\n"); break;
  }
#undef Case
  /* Now we are using the stack for Allocate calls */
  circle = Circle_(3, 4, 5);

  /* define multiple interfaces at once: */
  ss = CircleWriterShape_(circle);
  printf("Area=%lf\n", ShapeArea(ss->shape));
  ShapeSetPosition(ss->shape, -3, -4);
  WriterWrite(ss->writer, stdout);
  printf("\n");

  /* Clear the stack */
  PagedStackClear(stack);

  circle = Circle_(9, 10, 15);

  /* define multiple interfaces at once: */
  ss = CircleWriterShape_(circle);
  printf("Area=%lf\n", ShapeArea(ss->shape));
  ShapeSetPosition(ss->shape, -3, -4);
  WriterWrite(ss->writer, stdout);
  printf("\n");
  return 0;
}
