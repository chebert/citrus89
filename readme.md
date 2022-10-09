Citrus89
--------

An experiment in adding macros to support high level language features like single-dispatch OOP, String Builders, efficient high-level allocators, tagged unions, and high level enum specifications.

One of the goals of Citrus89 is to generate compact, readable code, that is compliant with a C89 compiler.

# Documentation

Documentation is manually extracted from the code. For the most up to date documentation, please refer to the source code.


## Enum Specs

### Definition

```
/* An enum spec
 *   is named ENUM_<enum_name>
 *   takes a single parameter used to specify fields.
 *     The field parameter takes two arguments, field(enum_name, field_name)
 *
 * Enum(enum_name) generates
 *   an enum type with an EnumCount
 *   an <enum_name>ToString function
 *   an <enum_name>FromString function
 */
```

### Macro Expansion

```
/*
#define ENUM_Event(f)\
  f(Event, MouseMotion)\
  f(Event, MouseButton)\
  f(Event, Key)
Enum(Event)

*** Expansion ***
enum Event { Event_MouseMotion, Event_MouseButton, Event_Key, Event_COUNT };
String EventToString(enum Event field);
enum Event EventFromString(String name);
String EventToString(enum Event field) {
  static String names[] = { "MouseMotion", "MouseButton", "Key", };
  return field < Event_COUNT ? names[field] : "Invalid " "Event" " field"; }
enum Event EventFromString(String name) {
  static String names[] = { "MouseMotion", "MouseButton", "Key", };
  int field;
  for (field = 0; field < Event_COUNT; ++field) {
    if (!strcmp(names[field], name)) return field;
  }
  return -1;
}
*/
```

### Example

```
/*
#define ENUM_Event(f)\
  f(Event, MouseMotion)\
  f(Event, MouseButton)\
  f(Event, Key)
Enum(Event)

String name = "MouseButton";
int event;
for (event = 0; event < EnumCount(Event); ++event) {
  printf("%d, %s\n", event, EventToString(event));
}
printf("%s is %d\n", name, EventFromString(name));

0, MouseMotion
1, MouseButton
2, Key
MouseButton is 1
 */
```


## Struct Specs

### Definition
```
/* A struct spec:
 *   is prefixed with STRUCT_
 *   takes a single field argument
 *     field takes two arguments: field(type, field_name)
 *
 *
 * Struct(struct_name) generates:
 *   a struct typedef and name
 *   an initializer function for initializing a structure
 *   a constructor function for allocating and initializing a structure
 */

```

### Expansion
```
/*
#define STRUCT_ExampleStruct(f)\
  f(int, a)\
  f(double, b)\
  f(MString, c)

Struct(ExampleStruct)

*** Expansion ***
typedef struct ExampleStruct *ExampleStruct;
struct ExampleStruct {
    int a;
    double b;
    MString c;
};
ExampleStruct _InitExampleStruct(int a, double b, MString c, ExampleStruct _result);
ExampleStruct _ExampleStruct(int a, double b, MString c, ...);
ExampleStruct _InitExampleStruct(int a, double b, MString c, ExampleStruct _result) {
    _result->a = a;
    _result->b = b;
    _result->c = c;
    return _result;
}
ExampleStruct _ExampleStruct(int a, double b, MString c, ...) {
    return _InitExampleStruct(a, b, c, (ExampleStruct)Allocate(sizeof(struct ExampleStruct)));
}
*/
```

### Example
```
/*
#define STRUCT_ExampleStruct(f)\
  f(int, a)\
  f(double, b)\
  f(MString, c)

Struct(ExampleStruct)

void *malloc(size_t num_bytes);
void *Allocate(size_t num_bytes) { return malloc(num_bytes); }

*** Code ***
ExampleStruct example = _ExampleStruct(3, 3.14159, "The quick brown fox");
printf("Example: %d, %lf, %s\n", example->a, example->b, example->c);

*** Run Result ***
Example: 3, 3.141590, The quick brown fox
 */
```

## Interface

### Definition
```
/* An interface spec:
 *   is prefixed with INTERFACE_
 *   takes a single argument to specify methods
 *     each method takes two arguments: method(interface_name, method_name)
 * A method spec:
 *   is prefixed with METHOD_
 *   takes two arguments: (return_type, parameter)
 *     return_type takes a single argument: return_type(type)
 *     parameter takes two arguments: parameter(parameter_type, parameter_name)
 * 
 * Interface(interface_name) generates:
 *   An interface method table struct
 *   An interface struct
 *   Dispatch functions for each method in the interface
 */
```

### Expansion
```
/*
#define INTERFACE_Shape(m) m(Shape, Area) m(Shape, SetPosition)
#define METHOD_Shape_Area(r, p) r(f8)
#define METHOD_Shape_SetPosition(r, p) r(int) p(f8, x) p(f8, y)
Interface(Shape)

*** Expansion ***
typedef struct Shape_MethodTable *Shape_MethodTable;
struct Shape_MethodTable {
    f8 (*Area)(void * _object );
    int (*SetPosition)(void * _object, f8 x, f8 y);
};
typedef struct Shape *Shape;
struct Shape {
    Shape_MethodTable method_table;
    Any data;
};
f8 ShapeArea(Shape _interface );
int ShapeSetPosition(Shape _interface, f8 x, f8 y);
f8 ShapeArea(Shape _interface ) {
    return _interface->method_table->Area(_interface->data );
}
int ShapeSetPosition(Shape _interface, f8 x, f8 y) {
    return _interface->method_table->SetPosition(_interface->data, x, y);
}
*/

```

## Methods
### Definition
```
/* Methods are defined for objects using
 *   Method(object_type, interface_name, method_name)
 *      method body
 *   EndMethod
 *   Within the method body:
 *     self is bound to the interface data, cast to an object_type
 *     parameters are bound to the names specified by the method spec
 */
```

### Expansion
```
/*
#define INTERFACE_Shape(m) m(Shape, Area) m(Shape, SetPosition)
#define METHOD_Shape_Area(r, p) r(f8)
#define METHOD_Shape_SetPosition(r, p) r(int) p(f8, x) p(f8, y)

Method(Circle, Shape, Area)
  return self->radius * self->radius * 3.14159;
EndMethod

*** Expansion ***
f8 CircleShapeArea(void * _data ) {
    Circle self = (Circle)_data;
    return self->radius * self->radius * 3.14159;
}
*/
```

## Objects

### Definition
```
/* 
 * ObjectConstructors(interface) generates
 *   An Init function which takes an object and an interface
 *   A Make function which takes an object and allocates a new interface
 * OBJECT is expected to be defined to be the object type
 */

/* An interface is defined using:
 *   an interface spec
 *   method specs for each method in the interface
 *   Interface(interface_name)
 *
 * An object implements an interface by:
 *   implementing methods using Method(...) ... EndMethod 
 *   implementing constructors using ObjectConstructors(...)
 */
```

### Expansion
```
/*
#define INTERFACE_Shape(m) m(Shape, Area) m(Shape, SetPosition)

#define OBJECT Circle
ObjectConstructors(Shape)
#undef OBJECT

*** Expansion ***
Shape InitCircleShape(Circle _object, Shape _interface) {
    static struct Shape_MethodTable _method_table = { CircleShapeArea, CircleShapeSetPosition, };
    _interface->method_table = &_method_table;
    _interface->data = (Any)_object;
    return _interface;
}
Shape CircleShape(Circle _object) {
    return InitCircleShape(_object, (Shape)Allocate(sizeof(struct Shape)));
}
 */
```

### Example
```
 /*
#define INTERFACE_Shape(m) m(Shape, Area) m(Shape, SetPosition)
#define METHOD_Shape_Area(r, p) r(f8)
#define METHOD_Shape_SetPosition(r, p) r(int) p(f8, x) p(f8, y)
Interface(Shape)

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
  return 0;
EndMethod

#define OBJECT Circle
ObjectConstructors(Shape)
#undef OBJECT

void *malloc(size_t num_bytes);
void *Allocate(size_t num_bytes) { return malloc(num_bytes); }


Circle circle = _Circle(3, 4, 5);
Shape shape = CircleShape(circle);
printf("%lf, <%lf, %lf>\n", ShapeArea(shape), circle->x, circle->y);
ShapeSetPosition(shape, 8, 9);
printf("%lf, <%lf, %lf>\n", ShapeArea(shape), circle->x, circle->y);

78.539750, <3.000000, 4.000000>
78.539750, <8.000000, 9.000000>
  */
```


## Interface Composition

### Definiton
```
/* A composite interface is defined using:
 *   interface specs
 *   a composite spec which is a structure of interfaces
 *   DefineCompositeConstructor(composite) with OBJECT defined to be the name of the implementing object.
 */
```

### Example Usage
```
/*

#define INTERFACE_Shape(m) m(Shape, Area)
#define INTERFACE_Write(m) m(Write, Write)
#define STRUCT_ShapeWriter(f) f(Shape, shape) f(Write, write)
DeclareCompositeConstructor(Example, ShapeWriter)
#define OBJECT Example
DefineCompositeConstructor(ShapeWriter)
#undef OBJECT

 */
```


## Tagged Unions

### Definiton
```
/* A tagged union spec
 *   is prefixed with TAGGED_UNION_
 *   takes a single parameter for specifying fields
 *     each field takes 3 arguments: field(tagged_union_name, field_type, field_name)
 *
 * TaggedUnion(tagged_union_name) generates:
 *   an enum type for tags
 *   a struct type with a tag and a union field for each tagged union field type
 *   constructors and initializers for each field
 */
```

### Expansion
```
/*
#define TAGGED_UNION_Example(f)\
  f(Example, s4, Integer)\
  f(Example, f8, Double)\
  f(Example, Any, Object)
TaggedUnion(Example)

*** Expansion ***
enum Example_Tag { Example_Tag_Integer, Example_Tag_Double, Example_Tag_Object, Example_Tag_COUNT };
typedef struct Example *Example;
struct Example {
    enum Example_Tag tag;
    union {
        s4 Integer;
        f8 Double;
        Any Object;
    } data;
};
typedef s4 TAGGED_UNION_FIELD_TYPE_Example_Integer;
typedef f8 TAGGED_UNION_FIELD_TYPE_Example_Double;
typedef Any TAGGED_UNION_FIELD_TYPE_Example_Object;
Example InitExampleInteger(s4 _field, Example _result);
Example InitExampleDouble(f8 _field, Example _result);
Example InitExampleObject(Any _field, Example _result);
Example ExampleInteger(s4 _field);
Example ExampleDouble(f8 _field);
Example ExampleObject(Any _field);
Example InitExampleInteger(s4 _field, Example _result) {
    _result->tag = Example_Tag_Integer;
    _result->data.Integer = _field;
    return _result;
}
Example InitExampleDouble(f8 _field, Example _result) {
    _result->tag = Example_Tag_Double;
    _result->data.Double = _field;
    return _result;
}
Example InitExampleObject(Any _field, Example _result) {
    _result->tag = Example_Tag_Object;
    _result->data.Object = _field;
    return _result;
}
Example ExampleInteger(s4 _field) {
    return InitExampleInteger(_field, (Example)Allocate(sizeof(struct Example)));
}
Example ExampleDouble(f8 _field) {
    return InitExampleDouble(_field, (Example)Allocate(sizeof(struct Example)));
}
Example ExampleObject(Any _field) {
    return InitExampleObject(_field, (Example)Allocate(sizeof(struct Example)));
}
*/
```

### Tagged Switch

#### Expansion
```
/*
TaggedSwitch(d) {
  TaggedCase(Example, d, Integer) printf("Integer: %d\n", Integer); EndCase
  TaggedCase(Example, d, Double) printf("Double: %lf\n", Double); EndCase
  TaggedCase(Example, d, Object) printf("Object: %p\n", Object); EndCase
}

*** Expansion ***
switch (d->tag) {
case Example_Tag_Integer: {
    TAGGED_UNION_FIELD_TYPE_Example_Integer Integer = d->data.Integer;
    printf("Integer: %d\n", Integer);
}
break;
case Example_Tag_Double: {
    TAGGED_UNION_FIELD_TYPE_Example_Double Double = d->data.Double;
    printf("Double: %lf\n", Double);
}
break;
case Example_Tag_Object: {
    TAGGED_UNION_FIELD_TYPE_Example_Object Object = d->data.Object;
    printf("Object: %p\n", Object);
}
break;
}
*/
```

#### Example
```
/*
void *malloc(size_t num_bytes);
void *Allocate(size_t num_bytes) { return malloc(num_bytes); }

*** Code ***
Example d = ExampleDouble(3.14159);
TaggedSwitch(d) {
  TaggedCase(Example, d, Integer) printf("Integer: %d\n", Integer); EndCase
  TaggedCase(Example, d, Double) printf("Double: %lf\n", Double); EndCase
  TaggedCase(Example, d, Object) printf("Object: %p\n", Object); EndCase
}
*** Run Result ***
Double: 3.141590
 */
 ```

## Allocators

Allocators are implemented behind an Allocator interface.
A jump buffer can be prepared using `CatchAllocatorOutOfMemoryError {}`. Allocators are expected to throw to the `global_allocator_jump_buffer` in the event of a failure to allocate memory.

### Malloc Allocator
Malloc allocator uses the C89 runtime's implementation of malloc to allocate memory.

### Pool
```
Pool InitPool(size_t element_size, Allocator element_allocator, Pool pool);
void *PoolAllocate(Pool pool);
void PoolFree(Pool pool, void *element);
```

A pool uses the provided `element_allocator` to allocate elements of a fixed size. Freed elements are retained in a linked list for fast re-allocation.

### Dynamic Stack
```
Stack MakeStack(u1 *buffer_bytes, size_t buffer_size);
Stack InitStack(u1 *buffer_bytes, size_t buffer_size, Stack stack);

u1 *StackAllocate(Stack stack, size_t num_bytes);
void StackAlign(Stack stack, size_t num_bytes);
u1 *StackClear(Stack stack);
```

A dynamic stack uses the global allocator to allocate memory to pages. If there isn't enough memory in the current page, a new page is allocated.
Stack allocations are O(1) if there is room on the current page, otherwise the time complexity is determined by the global allocator.
Freeing the stack is O(p) where p is the number of pages allocated (much faster than O(a) where a is the number of allocations).


## StringBuffer
```
String VSPrintf(size_t *length, String format, va_list args);
String SPrintf(size_t *length, String fmt, ...);
```

In theory, these could be implemented in C89. In practice, these are implemented using `snprintf`.

```
StringBuffer MakeStringBuffer(Allocator allocator);
StringBuffer InitStringBuffer(Allocator allocator, StringBuffer string_buffer);

/* Append to the end of the string buffer. */
void StringBufferPrintf(StringBuffer string_buffer, String format, ...);
/* Copy the StringBuffer to a single string, and clear the allocator. */
String StringBufferFinalize(StringBuffer string_buffer);
```

A StringBuffer uses the given allocator to hold a string that is being built over time. When finalized, string is copied using the global allocator, clearing the StringBuffer's allocator.
