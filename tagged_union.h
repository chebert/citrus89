#ifndef TAGGED_UNION_H
#define TAGGED_UNION_H

#define TAGGED_UNION_SPEC(name)\
  TAGGED_UNION_##name

#define TAGGED_UNION_FIELD_FOR_EACH(name, field)\
  TAGGED_UNION_SPEC(name)(field)

#define TAGGED_UNION_TAG(name, field_name)\
  TAGGED_UNION_TAG_##name##_##field_name
#define TAGGED_UNION_FIELD_TAG_NAME_COMMA_POSTFIXED(name, type, field_name)\
  TAGGED_UNION_TAG(name, field_name),
#define TAGGED_UNION_FIELD_DECLARATION(name, type, field_name)\
  type field_name;

#define TAGGED_UNION_FIELD_TYPE_NAME(name, field_name)\
  TAGGED_UNION_TYPE_##name##_##field_name
#define TAGGED_UNION_FIELD_TYPEDEF(name, type, field_name)\
  typedef type TAGGED_UNION_FIELD_TYPE_NAME(name, field_name);

#define TAGGED_UNION_ENUM_TYPE_DECLARATION(name)\
  enum Tag_##name {\
    TAGGED_UNION_FIELD_FOR_EACH(name, TAGGED_UNION_FIELD_TAG_NAME_COMMA_POSTFIXED)\
    TAGGED_UNION_TAG(name, COUNT)\
  };

#define TAGGED_UNION_STRUCT_TYPE_DECLARATION(name)\
  struct name {\
    enum Tag_##name tag;\
    union {\
      TAGGED_UNION_FIELD_FOR_EACH(name, TAGGED_UNION_FIELD_DECLARATION)\
    } data;\
  };\
  typedef struct name *name;

#define TAGGED_UNION_FIELD_INITIALIZER_DECLARATION(name, type, field_name)\
  name _##name##field_name(type field, name result)
#define TAGGED_UNION_FIELD_INITIALIZER_DEFINITION(name, type, field_name)\
  TAGGED_UNION_FIELD_INITIALIZER_DECLARATION(name, type, field_name) {\
    result = result ? result : AllocateStruct(name);\
    result->tag = TAGGED_UNION_TAG(name, field_name);\
    result->data.field_name = field;\
    return result;\
  }
#define TAGGED_UNION_FIELD_INITIALIZER_DECLARATION_TERMINATED(name, type, field_name)\
  TAGGED_UNION_FIELD_INITIALIZER_DECLARATION(name, type, field_name);

#define TAGGED_UNION_FIELD_CONSTRUCTOR_DECLARATION(name, type, field_name)\
  name name##field_name##_(type field)
#define TAGGED_UNION_FIELD_CONSTRUCTOR_DEFINITION(name, type, field_name)\
  TAGGED_UNION_FIELD_CONSTRUCTOR_DECLARATION(name, type, field_name) {\
    return _##name##field_name(field, NULL);\
  }
#define TAGGED_UNION_FIELD_CONSTRUCTOR_DECLARATION_TERMINATED(name, type, field_name)\
  TAGGED_UNION_FIELD_CONSTRUCTOR_DECLARATION(name, type, field_name);

#define TAGGED_UNION_INITIALIZER_DECLARATIONS(name)\
  TAGGED_UNION_FIELD_FOR_EACH(name, TAGGED_UNION_FIELD_INITIALIZER_DECLARATION_TERMINATED)
#define TAGGED_UNION_CONSTRUCTOR_DECLARATIONS(name)\
  TAGGED_UNION_FIELD_FOR_EACH(name, TAGGED_UNION_FIELD_CONSTRUCTOR_DECLARATION_TERMINATED)

#define TAGGED_UNION_INITIALIZER_DEFINITIONS(name)\
  TAGGED_UNION_FIELD_FOR_EACH(name, TAGGED_UNION_FIELD_INITIALIZER_DEFINITION)
#define TAGGED_UNION_CONSTRUCTOR_DEFINITIONS(name)\
  TAGGED_UNION_FIELD_FOR_EACH(name, TAGGED_UNION_FIELD_CONSTRUCTOR_DEFINITION)

 
#define TAGGED_UNION_DECLARATION(name)\
  TAGGED_UNION_ENUM_TYPE_DECLARATION(name)\
  TAGGED_UNION_STRUCT_TYPE_DECLARATION(name)\
  TAGGED_UNION_FIELD_FOR_EACH(name, TAGGED_UNION_FIELD_TYPEDEF)\
  TAGGED_UNION_INITIALIZER_DECLARATIONS(name)\
  TAGGED_UNION_CONSTRUCTOR_DECLARATIONS(name)

#define TAGGED_UNION_DEFINITION(name)\
  TAGGED_UNION_INITIALIZER_DEFINITIONS(name)\
  TAGGED_UNION_CONSTRUCTOR_DEFINITIONS(name)

#define DeclareTaggedUnion(name) TAGGED_UNION_DECLARATION(name)
#define DefineTaggedUnion(name) TAGGED_UNION_DEFINITION(name)
#define TaggedUnion(name)\
  DeclareTaggedUnion(name)\
  DefineTaggedUnion(name)

#define TaggedSwitch(value) switch (value->tag)
#define TaggedCase(value, name, field_name)\
  case TAGGED_UNION_TAG(name, field_name): {\
    TAGGED_UNION_FIELD_TYPE_NAME(name, field_name) field_name = value->data.field_name;
#define EndCase } break

#endif
