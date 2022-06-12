#ifndef CITRUS89
#define CITRUS89

#include <stdint.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

/* Boolean */
typedef s8 b8;

#ifndef false
#define false 0
#endif
#ifndef true
#define true (!false)
#endif

typedef float f4;
typedef double f8;

typedef void *Any;

typedef char *MString;
typedef const MString String;

void *Allocate(size_t num_bytes);
void Free(void *allocation);
void ClearAllocator();

/* Enums */

#define EnumSpec(enum_name) ENUM_##enum_name
#define EnumField(enum_name, field_name) enum_name##_##field_name
#define EnumTypeField(enum_name, field_name) EnumField(enum_name, field_name),
#define EnumTypeFields(enum_spec) enum_spec(EnumTypeField)
#define EnumCount(enum_name) EnumField(enum_name, COUNT)

#define EnumFieldName(enum_name, field_name) #field_name
#define EnumFieldNameCommaPostfixed(enum_name, field_name) EnumFieldName(enum_name, field_name),
#define EnumFieldNames(enum_spec) enum_spec(EnumFieldNameCommaPostfixed)

#define DeclareEnumType(enum_spec, enum_name)\
  enum enum_name {\
    EnumTypeFields(enum_spec)\
    EnumCount(enum_name)\
  };

#define DeclareEnumToString(enum_name)\
  String enum_name##ToString(enum enum_name field)
#define DeclareEnumFromString(enum_name)\
  enum enum_name enum_name##FromString(String name)

#define DeclareEnum(enum_name)\
  DeclareEnumType(EnumSpec(enum_name), enum_name)\
  DeclareEnumToString(enum_name);\
  DeclareEnumFromString(enum_name);

#define DefineEnumToString(enum_spec, enum_name)\
  DeclareEnumToString(enum_name) {\
    static String names[] = { EnumFieldNames(enum_spec) };\
    return field < EnumCount(enum_name) ? names[field] : "Invalid " #enum_name " field";\
  }
#define DefineEnumFromString(enum_spec, enum_name)\
  DeclareEnumFromString(enum_name) {\
    static String names[] = { EnumFieldNames(enum_spec) };\
    int field;\
    for (field = 0; field < EnumCount(enum_name); ++field) {\
      if (!strcmp(names[field], name))\
        return field;\
    }\
    return -1;\
  }

#define DefineEnum(enum_name)\
  DefineEnumToString(EnumSpec(enum_name), enum_name)\
  DefineEnumFromString(EnumSpec(enum_name), enum_name)

#define Enum(enum_name)\
  DeclareEnum(enum_name)\
  DefineEnum(enum_name)

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

/* Structs */

#define StructTypeField(type, name) type name;
#define StructTypeFields(struct_spec) struct_spec(StructTypeField)
#define DeclareStructType(struct_spec, struct_name)\
  struct struct_name { StructTypeFields(struct_spec) };

#define Typedef(struct_name) typedef struct struct_name *struct_name;
#define Sizeof(struct_name) sizeof(struct struct_name)

#define StructSpec(struct_name) STRUCT_##struct_name

#define StructFieldParameter(type, name) type name,
#define StructFieldParameters(struct_spec) struct_spec(StructFieldParameter)

#define StructFieldArgumentCommaPrefixed(type, name) , name
#define StructFieldArgumentsCommaPrefixed(struct_spec) struct_spec(StructFieldArgumentCommaPrefixed)
#define StructFieldArgumentCommaPostfixed(type, name) name,
#define StructFieldArgumentsCommaPostfixed(struct_spec) struct_spec(StructFieldArgumentCommaPostfixed)

#define StructFieldResultAssignment(type, name) _result->name = name;
#define StructFieldResultAssignments(struct_spec) struct_spec(StructFieldResultAssignment)

#define DeclareStructInit(struct_spec, struct_name)\
  struct_name _Init##struct_name(StructFieldParameters(struct_spec) struct_name _result)
#define DefineStructInit(struct_spec, struct_name)\
  DeclareStructInit(struct_spec, struct_name) {\
    StructFieldResultAssignments(struct_spec)\
    return _result;\
  }

#define StructAllocate(struct_name) (struct_name)Allocate(sizeof(struct struct_name))

#define DeclareStructMake(struct_spec, struct_name)\
  struct_name _##struct_name(StructFieldParameters(struct_spec) ...)
#define DefineStructMake(struct_spec, struct_name)\
  DeclareStructMake(struct_spec, struct_name) {\
    return _Init##struct_name(StructFieldArgumentsCommaPostfixed(struct_spec) StructAllocate(struct_name));\
  }

#define DeclareStruct(struct_name)\
  Typedef(struct_name)\
  DeclareStructType(StructSpec(struct_name), struct_name)\
  DeclareStructInit(StructSpec(struct_name), struct_name);\
  DeclareStructMake(StructSpec(struct_name), struct_name);
#define DefineStruct(struct_name)\
  DefineStructInit(StructSpec(struct_name), struct_name)\
  DefineStructMake(StructSpec(struct_name), struct_name)

#define Struct(struct_name)\
  DeclareStruct(struct_name)\
  DefineStruct(struct_name)

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


/* Interfaces */
#define InterfaceSpec(interface) INTERFACE_##interface
#define MethodSpec(interface, method) METHOD_##interface##_##method

#define MethodReturnType(v) v
#define IgnoreMethodReturnType(v)

#define IgnoreMethodParameter(type, name)
#define MethodParameterCommaPrefixed(type, name) , type name
#define MethodArgumentCommaPrefixed(type, name) , name

#define InterfaceMethodDispatchFunctionName(interface, method)\
  interface##method

#define InterfaceMethodFunctionDeclaration(interface, method, name, object_type, object_name)\
  MethodSpec(interface, method)(MethodReturnType, IgnoreMethodParameter)\
    name(object_type object_name MethodSpec(interface, method)(IgnoreMethodReturnType, MethodParameterCommaPrefixed))

#define InterfaceMethodDispatchFunctionDeclaration(interface, method)\
  InterfaceMethodFunctionDeclaration(interface, method, InterfaceMethodDispatchFunctionName(interface, method), interface, _interface)

#define InterfaceMethodDispatchFunctionDeclarationTerminated(interface, method)\
  InterfaceMethodDispatchFunctionDeclaration(interface, method);

#define InterfaceMethodDispatchFunctionDefinition(interface, method)\
  InterfaceMethodDispatchFunctionDeclaration(interface, method) {\
    return _interface->method_table->method(_interface->data MethodSpec(interface, method)(IgnoreMethodReturnType, MethodArgumentCommaPrefixed));\
  }

#define InterfaceMethodTableFieldDeclaration(interface, method)\
  InterfaceMethodFunctionDeclaration(interface, method, (*method), void *, _object);

#define InterfaceMethodTableFieldDeclarations(interface_spec)\
  interface_spec(InterfaceMethodTableFieldDeclaration)

#define InterfaceMethodTableName(interface) interface##_MethodTable
#define DeclareInterfaceMethodTable(interface, interface_spec)\
  Typedef(InterfaceMethodTableName(interface))\
  struct InterfaceMethodTableName(interface) {\
    InterfaceMethodTableFieldDeclarations(interface_spec)\
  };

#define DeclareInterfaceType(interface)\
  Typedef(interface)\
  struct interface {\
    InterfaceMethodTableName(interface) method_table;\
    Any data;\
  };


#define DeclareInterfaceMethods(interface_spec)\
  interface_spec(InterfaceMethodDispatchFunctionDeclarationTerminated)
#define DefineInterfaceMethods(interface_spec)\
  interface_spec(InterfaceMethodDispatchFunctionDefinition)

#define DeclareInterface(interface)\
  DeclareInterfaceMethodTable(interface, InterfaceSpec(interface))\
  DeclareInterfaceType(interface)\
  DeclareInterfaceMethods(InterfaceSpec(interface))

#define DefineInterface(interface)\
  DefineInterfaceMethods(InterfaceSpec(interface))

#define Interface(interface)\
  DeclareInterface(interface)\
  DefineInterface(interface)

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

/* Methods */
#define DeclareMethod(object, interface, method)\
  InterfaceMethodFunctionDeclaration(interface, method, object##interface##method, void *, _data)

#define Method(object, interface, method)\
  DeclareMethod(object, interface, method) {\
    object self = (object)_data;
#define EndMethod }

/* Methods are defined for objects using
 *   Method(object_type, interface_name, method_name)
 *      method body
 *   EndMethod
 *   Within the method body:
 *     self is bound to the interface data, cast to an object_type
 *     parameters are bound to the names specified by the method spec
 */

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

/* Object Interface Constructors */

#define DeclareMakeObject(object, interface)\
  interface object##interface(object _object)
#define InitObjectInterfaceName(object, interface) Init##object##interface
#define DefineMakeObject(object, interface)\
  DeclareMakeObject(object, interface) {\
    return InitObjectInterfaceName(object, interface)(_object, StructAllocate(interface));\
  }

#define _ObjectMethodName(object, interface, method) object##interface##method
#define ObjectMethodName(object, interface, method) _ObjectMethodName(object, interface, method)
#define ObjectMethodNameCommaPostfixed(interface, method) ObjectMethodName(OBJECT, interface, method),
#define DeclareInitObject(object, interface)\
  interface InitObjectInterfaceName(object, interface)(object _object, interface _interface)
#define DefineInitObject(object, interface, interface_spec)\
  DeclareInitObject(object, interface) {\
    static struct InterfaceMethodTableName(interface) _method_table = {\
      interface_spec(ObjectMethodNameCommaPostfixed)\
    };\
    _interface->method_table = &_method_table;\
    _interface->data = (Any)_object;\
    return _interface;\
  }

#define DeclareObjectConstructors(object, interface)\
  DeclareMakeObject(object, interface);\
  DeclareInitObject(object, interface);

#define ObjectConstructors(interface)\
  DefineInitObject(OBJECT, interface, InterfaceSpec(interface))\
  DefineMakeObject(OBJECT, interface)

/* 
 * ObjectConstructors(interface) generates
 *   An Init function which takes an object and an interface
 *   A Make function which takes an object and allocates a new interface
 * OBJECT is expected to be defined to be the object type
 */

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

/* An interface is defined using:
 *   an interface spec
 *   method specs for each method in the interface
 *   Interface(interface_name)
 *
 * An object implements an interface by:
 *   implementing methods using Method(...) ... EndMethod 
 *   implementing constructors using ObjectConstructors(...)
 */

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

/* Compose Interfaces */

#define _MakeInterfaceName(object, interface) object##interface
#define MakeInterfaceName(object, interface) _MakeInterfaceName(object, interface)
#define MakeCompositeInterfaceCommaPostfixed(interface, name)\
  MakeInterfaceName(OBJECT, interface)(object),

#define InitCompositeName(object, composite) Init##object##composite
#define DeclareInitComposite(object_name, composite)\
  composite InitCompositeName(object_name, composite)(object_name object, composite _composite)
#define DefineInitComposite(object_name, composite)\
  DeclareInitComposite(object_name, composite) {\
    return _Init##composite(StructSpec(composite)(MakeCompositeInterfaceCommaPostfixed) _composite);\
  }

#define DeclareMakeComposite(object_name, composite)\
  composite object_name##composite(object_name object)
#define DefineMakeComposite(object_name, composite)\
  DeclareMakeComposite(object_name, composite) {\
    return InitCompositeName(object_name, composite)(object, StructAllocate(composite));\
  }

#define DeclareCompositeConstructor(object, composite)\
  DeclareInitComposite(object, composite);\
  DeclareMakeComposite(object, composite);

#define DefineCompositeConstructor(composite)\
  DefineInitComposite(OBJECT, composite)\
  DefineMakeComposite(OBJECT, composite)

/*

#define INTERFACE_Shape(m) m(Shape, Area)
#define INTERFACE_Write(m) m(Write, Write)
#define STRUCT_ShapeWriter(f) f(Shape, shape) f(Write, write)
DeclareCompositeConstructor(Example, ShapeWriter)
#define OBJECT Example
DefineCompositeConstructor(ShapeWriter)
#undef OBJECT

 */

/* A composite interface is defined using:
 *   interface specs
 *   a composite spec which is a structure of interfaces
 *   DefineCompositeConstructor(composite) with OBJECT defined to be the name of the implementing object.
 */

/* Tagged Unions */

#define TaggedUnionSpec(tagged_union_name) TAGGED_UNION_##tagged_union_name

#define TaggedUnionEnumTypeName(tagged_union_name) tagged_union_name##_Tag

#define TaggedUnionEnumField(tagged_union_enum_name, field_name)\
  EnumField(tagged_union_enum_name, field_name)

#define TaggedUnionEnumTypeField(tagged_union_name, field_type, field_name)\
  EnumTypeField(TaggedUnionEnumTypeName(tagged_union_name), field_name)
#define TaggedUnionEnumTypeFields(tagged_union_spec)\
  tagged_union_spec(TaggedUnionEnumTypeField)
#define TaggedUnionEnumTypeDeclaration(tagged_union_name)\
  enum TaggedUnionEnumTypeName(tagged_union_name) {\
    TaggedUnionEnumTypeFields(TaggedUnionSpec(tagged_union_name))\
    EnumCount(TaggedUnionEnumTypeName(tagged_union_name))\
  };

#define TaggedUnionUnionTypeField(tagged_union_name, field_type, field_name)\
  field_type field_name;
#define TaggedUnionUnionTypeFields(tagged_union_spec)\
  tagged_union_spec(TaggedUnionUnionTypeField)

#define TaggedUnionStructTypeDeclaration(tagged_union_name)\
  Typedef(tagged_union_name)\
  struct tagged_union_name {\
    enum TaggedUnionEnumTypeName(tagged_union_name) tag;\
    union {\
      TaggedUnionUnionTypeFields(TaggedUnionSpec(tagged_union_name))\
    } data;\
  };

#define TaggedUnionFieldTypeName(tagged_union_name, field_name)\
  TAGGED_UNION_FIELD_TYPE##_##tagged_union_name##_##field_name
#define TaggedUnionFieldTypedef(tagged_union_name, field_type, field_name)\
  typedef field_type TaggedUnionFieldTypeName(tagged_union_name, field_name);
#define TaggedUnionFieldTypedefs(tagged_union_name)\
  TaggedUnionSpec(tagged_union_name)(TaggedUnionFieldTypedef)

#define TaggedUnionInitDeclaration(tagged_union_name, field_type, field_name)\
  tagged_union_name Init##tagged_union_name##field_name(field_type _field, tagged_union_name _result)
#define TaggedUnionInitDefinition(tagged_union_name, field_type, field_name)\
  TaggedUnionInitDeclaration(tagged_union_name, field_type, field_name) {\
    _result->tag = TaggedUnionEnumField(TaggedUnionEnumTypeName(tagged_union_name), field_name);\
    _result->data.field_name = _field;\
    return _result;\
  }

#define TaggedUnionInitDeclarationTerminated(tagged_union_name, field_type, field_name)\
  TaggedUnionInitDeclaration(tagged_union_name, field_type, field_name);
#define TaggedUnionInitDeclarations(tagged_union_name)\
  TaggedUnionSpec(tagged_union_name)(TaggedUnionInitDeclarationTerminated)
#define TaggedUnionInitDefinitions(tagged_union_name)\
  TaggedUnionSpec(tagged_union_name)(TaggedUnionInitDefinition)

#define TaggedUnionMakeDeclaration(tagged_union_name, field_type, field_name)\
  tagged_union_name tagged_union_name##field_name(field_type _field)
#define TaggedUnionMakeDefinition(tagged_union_name, field_type, field_name)\
  TaggedUnionMakeDeclaration(tagged_union_name, field_type, field_name) {\
    return Init##tagged_union_name##field_name(_field, StructAllocate(tagged_union_name));\
  }
#define TaggedUnionMakeDeclarationTerminated(tagged_union_name, field_type, field_name)\
  TaggedUnionMakeDeclaration(tagged_union_name, field_type, field_name);

#define TaggedUnionMakeDeclarations(tagged_union_name)\
  TaggedUnionSpec(tagged_union_name)(TaggedUnionMakeDeclarationTerminated)

#define TaggedUnionMakeDefinitions(tagged_union_name)\
  TaggedUnionSpec(tagged_union_name)(TaggedUnionMakeDefinition)

#define DeclareTaggedUnion(tagged_union_name)\
  TaggedUnionEnumTypeDeclaration(tagged_union_name)\
  TaggedUnionStructTypeDeclaration(tagged_union_name)\
  TaggedUnionFieldTypedefs(tagged_union_name)\
  TaggedUnionInitDeclarations(tagged_union_name)\
  TaggedUnionMakeDeclarations(tagged_union_name)
#define DefineTaggedUnion(tagged_union_name)\
  TaggedUnionInitDefinitions(tagged_union_name)\
  TaggedUnionMakeDefinitions(tagged_union_name)
#define TaggedUnion(tagged_union_name)\
  DeclareTaggedUnion(tagged_union_name)\
  DefineTaggedUnion(tagged_union_name)


#define TaggedSwitch(tagged_union) switch (tagged_union->tag)
#define TaggedCase(tagged_union_name, tagged_union, field_name)\
  case TaggedUnionEnumField(TaggedUnionEnumTypeName(tagged_union_name), field_name): {\
    TaggedUnionFieldTypeName(tagged_union_name, field_name) field_name = tagged_union->data.field_name;
#define EndCase } break;

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

/* Allocators */

#define INTERFACE_Allocator(M)\
  M(Allocator, Allocate)\
  M(Allocator, Free)\
  M(Allocator, Clear)
#define METHOD_Allocator_Allocate(R, A) R(void *) A(size_t, num_bytes)
#define METHOD_Allocator_Free(R, A) R(int) A(void *, data)
#define METHOD_Allocator_Clear(R, A) R(int)
DeclareInterface(Allocator)

/* Global Allocator */
extern jmp_buf global_allocator_jump_buffer;
#define CatchGlobalAllocatorOutOfMemoryError\
  if (setjmp(global_allocator_jump_buffer))

void SetGlobalAllocator(Allocator allocator);

/* Malloc Allocator */
Allocator MallocAllocator();

Typedef(PoolFreeList)
#define STRUCT_Pool(f)\
  f(size_t, element_size)\
  f(Allocator, element_allocator)\
  f(PoolFreeList, next)
DeclareStruct(Pool)

/* Need a Allocator&Deallocator */
Pool MakePool(size_t element_size, Allocator element_allocator);
Pool InitPool(size_t element_size, Allocator element_allocator, Pool pool);

void *PoolAllocate(Pool pool);
void PoolFree(Pool pool, void *element);
/* TODO */
void PoolReset(Pool pool);

#define STRUCT_Stack(f)\
  f(u1 *, buffer_bytes)\
  f(size_t, buffer_size)\
  f(size_t, index)
DeclareStruct(Stack)

Stack MakeStack(u1 *buffer_bytes, size_t buffer_size);
Stack InitStack(u1 *buffer_bytes, size_t buffer_size, Stack stack);

u1 *StackAllocate(Stack stack, size_t num_bytes);
void StackAlign(Stack stack, size_t num_bytes);
u1 *StackClear(Stack stack);

Typedef(DynamicStackAllocationHeader)
#define STRUCT_DynamicStack(f)\
  f(Pool, page_pool)\
  f(DynamicStackAllocationHeader, last_allocation)\
  f(Stack, stack)
DeclareStruct(DynamicStack)

DynamicStack MakeDynamicStack(Pool page_pool);
DynamicStack InitDynamicStack(Pool page_pool, Stack stack, DynamicStack dynamic_stack);

void *DynamicStackAllocate(DynamicStack dynamic_stack, size_t num_bytes);
void DynamicStackAlign(DynamicStack dynamic_stack, size_t num_bytes);
void DynamicStackClear(DynamicStack dynamic_stack);

DeclareObjectConstructors(DynamicStack, Allocator)

#define SPRINTF_BUFFER_SIZE_IN_BYTES 4096
String VSPrintf(size_t *length, String format, va_list args);
String SPrintf(size_t *length, String fmt, ...);

/* StringBuffer */
Typedef(StringBufferString)
#define STRUCT_StringBuffer(f)\
  f(Allocator, allocator)\
  f(StringBufferString, first_string)\
  f(StringBufferString, last_string)\
  f(size_t, length)
DeclareStruct(StringBuffer)

StringBuffer MakeStringBuffer(Allocator allocator);
StringBuffer InitStringBuffer(Allocator allocator, StringBuffer string_buffer);

/* Append to the end of the string buffer. */
void StringBufferPrintf(StringBuffer string_buffer, String format, ...);
/* Copy the StringBuffer to a single string, and clear the allocator. */
String StringBufferFinalize(StringBuffer string_buffer);

#endif
