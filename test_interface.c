#include "interface.h"

#include <stdio.h>

#include "global_allocator.h"
#include "struct.h"
#include "page_size.h"
#include "paged_stack.h"

#define INTERFACE_Shape(m)\
  m(Shape, Area)\
  m(Shape, SetPosition)
#define METHOD_Shape_Area(r, a) r(double)
#define METHOD_Shape_SetPosition(r, a) r(void) a(double, x) a(double, y)
Interface(Shape);

#define METHOD_Serialize_ToString(r, a) r(String)
#define INTERFACE_Serialize(m) m(Serialize, ToString)
Interface(Serialize);

#define STRUCT_Circle(f)\
  f(double, x)\
  f(double, y)\
  f(double, radius)
Struct(Circle);

Method(Circle, Shape, Area)
  return self->radius * self->radius * 3.14159;
EndMethod
Method(Circle, Shape, SetPosition)
  self->x = x;
  self->y = y;
EndMethod

Method(Circle, Serialize, ToString)
  MString string = Allocate(256);
  snprintf(string, 256, "Circle { .x=%lf, .y=%lf, .radius=%lf }", self->x, self->y, self->radius);
  return string;
EndMethod

// A composite interface is just a struct with interfaces for fields
#define STRUCT_SerializableShape(f)\
  f(Serialize, serialize)\
  f(Shape, shape)
Struct(SerializableShape);

#define OBJECT Circle
DefineConstructor(Shape);
DefineConstructor(Serialize);
DefineCompositeConstructor(SerializableShape);
#undef OBJECT

MethodDispatchFunctionAlias(Serialize, ToString, ToString);

// TODO: move these allocators
Method(PagedStack, Allocator, Allocate)
  return PagedStackAllocate(self, num_bytes);
EndMethod

#define OBJECT PagedStack
DefineConstructor(Allocator);
#undef OBJECT

Method(Stack, Allocator, Allocate)
  return StackAllocate(self, num_bytes);
EndMethod

#define OBJECT Stack
DefineConstructor(Allocator);
#undef OBJECT

// NOTE:
// Init: _Type(type-field-arguments..., Type storage) => Type
// Make: Type_(type-field-arguments...) => newly allocated Type

#define STATIC_MEMORY_SIZE_IN_BYTES 1024
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

#define QUOTE(...) #__VA_ARGS__
#define PRINT(macro_call) printf("\"%s\" expands to \"%s\"\n", #macro_call, QUOTE(macro_call))



int main(int argc, char **argv) {
  CatchGlobalAllocatorOutOfMemoryError {
    // Run this code if we ever run out of memory.
    fprintf(stderr, "Out of memory error\n");
    return -1;
  }
  // Use a static stack to allocate
  SetGlobalAllocator(StaticAllocator());

  PagedStack stack = MakePagedStack(MakePagePool(GetPageSizeInBytesFromOS()));
  SetGlobalAllocator(PagedStackAllocator(stack));

  // Now we are using the stack for Allocate calls
  Circle circle = Circle_(3, 4, 5);

  // define multiple interfaces at once:
  SerializableShape ss = CircleSerializableShape_(circle);
  printf("Area=%lf\n", ShapeArea(ss->shape));
  ShapeSetPosition(ss->shape, -3, -4);
  printf("%s\n", ToString(ss->serialize));

  // Clear the stack
  PagedStackClear(stack);

  circle = Circle_(9, 10, 15);

  // define multiple interfaces at once:
  ss = CircleSerializableShape_(circle);
  printf("Area=%lf\n", ShapeArea(ss->shape));
  ShapeSetPosition(ss->shape, -3, -4);
  printf("%s\n", ToString(ss->serialize));
  return 0;
}
