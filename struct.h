#ifndef STRUCT_H
#define STRUCT_H

#include "allocate.h"

#define STRUCT_FOR_EACH_FIELD(struct_name, macro)\
  STRUCT_##struct_name(macro)

#define STRUCT_FIELD_DECLARATION(type, name) type name;
#define STRUCT_FIELD_PARAMETER(type, name) type name,
#define STRUCT_FIELD_ARGUMENT(type, name) name,
#define STRUCT_FIELD_ASSIGN_RESULT(type, name) __result->name = name;

#define STRUCT_DECLARATION(struct_name)\
  struct struct_name { STRUCT_FOR_EACH_FIELD(struct_name, STRUCT_FIELD_DECLARATION) };

#define STRUCT_TYPEDEF(struct_name) typedef struct struct_name *struct_name;

#define STRUCT_INITIALIZER_NAME(struct_name) _##struct_name
#define STRUCT_CONSTRUCTOR_NAME(struct_name) struct_name##_

#define STRUCT_INITIALIZER_DECLARATION(struct_name)\
  struct_name STRUCT_INITIALIZER_NAME(struct_name)(STRUCT_FOR_EACH_FIELD(struct_name, STRUCT_FIELD_PARAMETER) struct_name __result_storage)

#define STRUCT_CONSTRUCTOR_DECLARATION(struct_name)\
  struct_name STRUCT_CONSTRUCTOR_NAME(struct_name)(STRUCT_FOR_EACH_FIELD(struct_name, STRUCT_FIELD_PARAMETER) ...)

#define STRUCT_INITIALIZER_DEFINITION(struct_name)\
  STRUCT_INITIALIZER_DECLARATION(struct_name) {\
    struct_name __result = __result_storage ? __result_storage : (struct_name)Allocate(sizeof(struct struct_name));\
    STRUCT_FOR_EACH_FIELD(struct_name, STRUCT_FIELD_ASSIGN_RESULT)\
    return __result;\
  }

#define STRUCT_CONSTRUCTOR_DEFINITION(struct_name)\
  STRUCT_CONSTRUCTOR_DECLARATION(struct_name) {\
    return STRUCT_INITIALIZER_NAME(struct_name)(STRUCT_FOR_EACH_FIELD(struct_name, STRUCT_FIELD_ARGUMENT) NULL);\
  }

#define DeclareStruct(struct_name)\
  STRUCT_TYPEDEF(struct_name)\
  STRUCT_DECLARATION(struct_name)\
  STRUCT_INITIALIZER_DECLARATION(struct_name);\
  STRUCT_CONSTRUCTOR_DECLARATION(struct_name);

#define DefineStruct(struct_name)\
  STRUCT_INITIALIZER_DEFINITION(struct_name)\
  STRUCT_CONSTRUCTOR_DEFINITION(struct_name)

#define Struct(struct_name)\
  DeclareStruct(struct_name)\
  DefineStruct(struct_name)

#define Sizeof(struct_name)\
  (sizeof(struct struct_name))

#define StructTypedef(struct_name)\
  STRUCT_TYPEDEF(struct_name)

#endif
