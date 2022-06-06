#ifndef ENUM_H
#define ENUM_H

#include "type.h"

#define ENUM_FIELDS_SPEC(enum_name)\
  ENUM_##enum_name
#define ENUM_FOR_EACH_FIELD(enum_name, field_do)\
  ENUM_FIELDS_SPEC(enum_name)(field_do)

#define ENUM_FIELD(enum_name, field_name)\
  ENUM_FIELD_##enum_name##_##field_name
#define ENUM_COUNT(enum_name) ENUM_FIELD(enum_name, COUNT)

#define ENUM_FIELD_NAME_COMMA_POSTFIXED(enum_name, field_name)\
  ENUM_FIELD(enum_name, field_name),
#define ENUM_FIELD_STRING_COMMA_POSTFIXED(enum_name, field_name) (String)(#enum_name ":" #field_name),

#define ENUM_TO_STRING_FUNCTION_NAME(enum_name)\
  ENUM_##enum_name##_TO_STRING
#define ENUM_TO_STRING_FUNCTION_DECLARATION(enum_name)\
  String ENUM_TO_STRING_FUNCTION_NAME(enum_name)(enum enum_name field_name)
#define ENUM_TO_STRING_FUNCTION_DEFINITION(enum_name)\
  ENUM_TO_STRING_FUNCTION_DECLARATION(enum_name) {\
    static String names[] = {\
      ENUM_FOR_EACH_FIELD(enum_name, ENUM_FIELD_STRING_COMMA_POSTFIXED)\
    };\
    size_t index = (size_t)field_name;\
    return index < ENUM_COUNT(enum_name)\
      ? names[(size_t)field_name]\
      : (String)"INVALID_ENUM_FIELD";\
  }

#define ENUM_TYPE_DECLARATION(enum_name)\
  enum enum_name {\
    ENUM_FOR_EACH_FIELD(enum_name, ENUM_FIELD_NAME_COMMA_POSTFIXED)\
    ENUM_COUNT(enum_name)\
  };


#define EnumField(enum_name, field_name)\
  ENUM_FIELD(enum_name, field_name)

#define EnumForEachField(enum_name, field)\
  ENUM_FOR_EACH_FIELD(enum_name, field)

#define EnumToString(enum_name, field)\
  (ENUM_TO_STRING_FUNCTION_NAME(enum_name)((enum enum_name)field))

#define EnumCount(enum_name) ENUM_FIELD(enum_name, COUNT)

#define DeclareEnum(enum_name)\
  ENUM_TYPE_DECLARATION(enum_name)\
  ENUM_TO_STRING_FUNCTION_DECLARATION(enum_name);
#define DefineEnum(enum_name)\
  ENUM_TO_STRING_FUNCTION_DEFINITION(enum_name)
#define Enum(enum_name)\
  DeclareEnum(enum_name)\
  DefineEnum(enum_name)

#endif
