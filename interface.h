#ifndef INTERFACE_H
#define INTERFACE_H

#include "allocate.h"

#define INTERFACE_SPEC(interface) INTERFACE_##interface

#define INTERFACE_METHOD_FOR_EACH(interface, do_method)\
  INTERFACE_SPEC(interface)(do_method)

#define METHOD_SPEC(interface, method) METHOD_##interface##_##method
#define RETURN_TYPE(type) type
#define RETURN_TYPE_IGNORED(type)
#define ARGUMENT_IGNORED(type, name)
#define ARGUMENT_COMMA_PREFIXED(type, name) , type name
#define ARGUMENT_NAME_COMMA_PREFIXED(type, name) , name

#define METHOD_ARGUMENTS_FOR_EACH(interface, method, do_argument)\
  METHOD_SPEC(interface, method)(RETURN_TYPE_IGNORED, do_argument)

#define METHOD_RETURN_TYPE(interface, method)\
  METHOD_SPEC(interface, method)(RETURN_TYPE, ARGUMENT_IGNORED)

#define METHOD_ARGUMENTS(interface, method)\
  METHOD_ARGUMENTS_FOR_EACH(interface, method, ARGUMENT_COMMA_PREFIXED)
#define METHOD_ARGUMENT_NAMES(interface, method)\
  METHOD_ARGUMENTS_FOR_EACH(interface, method, ARGUMENT_NAME_COMMA_PREFIXED)


#define INTERFACE_STRUCT_METHOD_DECLARATION(interface, method)\
  METHOD_RETURN_TYPE(interface, method) (*method)(void *data METHOD_ARGUMENTS(interface, method));

#define METHOD_DISPATCH_FUNCTION_DECLARATION(interface, method, dispatch_function)\
  METHOD_RETURN_TYPE(interface, method) dispatch_function(interface object METHOD_ARGUMENTS(interface, method))

#define DEFAULT_METHOD_DISPATCH_FUNCTION_DECLARATION(interface, method)\
  METHOD_DISPATCH_FUNCTION_DECLARATION(interface, method, interface##method)

#define METHOD_DISPATCH_FUNCTION_DECLARATION_TERMINATED(interface, method)\
  DEFAULT_METHOD_DISPATCH_FUNCTION_DECLARATION(interface, method);

#define METHOD_DISPATCH_FUNCTION_DEFINITION(interface_name, method)\
  DEFAULT_METHOD_DISPATCH_FUNCTION_DECLARATION(interface_name, method) {\
    object->interface->method(object->data METHOD_ARGUMENT_NAMES(interface_name, method));\
  }

#define INTERFACE_STRUCT_DECLARATION(interface, interface_struct_name)\
  struct interface_struct_name {\
    INTERFACE_METHOD_FOR_EACH(interface, INTERFACE_STRUCT_METHOD_DECLARATION)\
  };\
  typedef struct interface_struct_name *interface_struct_name;
#define INTERFACE_METHOD_DISPATCH_FUNCTION_DECLARATIONS(interface)\
  INTERFACE_METHOD_FOR_EACH(interface, METHOD_DISPATCH_FUNCTION_DECLARATION_TERMINATED)
#define INTERFACE_METHOD_DISPATCH_FUNCTION_DEFINITIONS(interface)\
  INTERFACE_METHOD_FOR_EACH(interface, METHOD_DISPATCH_FUNCTION_DEFINITION)

#define INTERFACE_OBJECT_STRUCT_DECLARATION(interface_name, interface_struct_name)\
  struct interface_name {\
    interface_struct_name interface;\
    void *data;\
  };\
  typedef struct interface_name *interface_name;

#define INTERFACE_DECLARATIONS(interface, interface_struct_name)\
  INTERFACE_STRUCT_DECLARATION(interface, interface_struct_name)\
  INTERFACE_OBJECT_STRUCT_DECLARATION(interface, interface_struct_name)\
  INTERFACE_METHOD_DISPATCH_FUNCTION_DECLARATIONS(interface)


#define INTERFACE_OBJECT_METHOD_NAME(object, interface, method) object##_##interface##_##method
#define OBJECT_METHOD_DECLARATION(object, interface, method)\
  METHOD_RETURN_TYPE(interface, method) INTERFACE_OBJECT_METHOD_NAME(object, interface, method)\
    (void *__data METHOD_ARGUMENTS(interface, method))

#define _INTERFACE_OBJECT_METHOD_NAME(object, interface, method)\
  INTERFACE_OBJECT_METHOD_NAME(object, interface, method)
/* extra indirection to expand OBJECT */
#define METHOD_OBJECT_NAME(interface, method)\
  _INTERFACE_OBJECT_METHOD_NAME(OBJECT, interface, method),

#define InterfaceStructName(interface) interface##_Interface

#define INTERFACE_OBJECT_INITIALIZER_DECLARATION(interface_name, object)\
  interface_name InterfaceObjectInitializerName(interface_name, object)(object data, interface_name result)
#define INTERFACE_OBJECT_INITIALIZER_DEFINITION(interface_name, object)\
  INTERFACE_OBJECT_INITIALIZER_DECLARATION(interface_name, object) {\
    static struct InterfaceStructName(interface_name) interface_methods = {\
      INTERFACE_METHOD_FOR_EACH(interface_name, METHOD_OBJECT_NAME)\
    };\
    result->interface = &interface_methods;\
    result->data = (void *)data;\
    return result;\
  }


#define INTERFACE_OBJECT_CONSTRUCTOR_DECLARATION(interface, object)\
  interface InterfaceObjectConstructorName(interface, object)(object data)
#define INTERFACE_OBJECT_CONSTRUCTOR_DEFINITION(interface, object)\
  INTERFACE_OBJECT_CONSTRUCTOR_DECLARATION(interface, object) {\
    return InterfaceObjectInitializerName(interface, object)(data, Allocate(sizeof(struct interface)));\
  }

#define INTERFACE_PASTE2(a, b) a##b
#define INTERFACE_COMPOSITE_OBJECT_CONSTRUCTOR_NAME(object, interface) INTERFACE_PASTE2(object,interface)
#define INTERFACE_COMPOSITE_OBJECT_CONSTRUCT(interface, name) INTERFACE_COMPOSITE_OBJECT_CONSTRUCTOR_NAME(OBJECT, interface)(object),

#define INTERFACE_COMPOSITE_INITIALIZER_NAME(object, interface) _##object##interface

#define INTERFACE_COMPOSITE_INITIALIZER_DECLARATION(object_name, interface)\
  interface INTERFACE_COMPOSITE_INITIALIZER_NAME(object_name, interface)(OBJECT object, interface interface_storage)

#define INTERFACE_COMPOSITE_INITIALIZER_DEFINITION(object_name, interface)\
  INTERFACE_COMPOSITE_INITIALIZER_DECLARATION(object_name, interface) {\
    return _##interface(\
        STRUCT_FOR_EACH_FIELD(interface, INTERFACE_COMPOSITE_OBJECT_CONSTRUCT)\
        interface_storage);\
  }

#define INTERFACE_COMPOSITE_CONSTRUCTOR_NAME(object, interface) object##interface##_
#define INTERFACE_COMPOSITE_CONSTRUCTOR_DECLARATION(object_name, interface)\
  interface INTERFACE_COMPOSITE_CONSTRUCTOR_NAME(object_name, interface)(OBJECT object)
#define INTERFACE_COMPOSITE_CONSTRUCTOR_DEFINITION(object_name, interface)\
  INTERFACE_COMPOSITE_CONSTRUCTOR_DECLARATION(object_name, interface) {\
    return INTERFACE_COMPOSITE_INITIALIZER_NAME(object_name, interface)(object, NULL);\
  }

#define DefineInterface(interface)\
  INTERFACE_METHOD_DISPATCH_FUNCTION_DEFINITIONS(interface)

#define DeclareInterface(interface)\
  INTERFACE_DECLARATIONS(interface, InterfaceStructName(interface))

#define Interface(interface)\
  DeclareInterface(interface)\
  DefineInterface(interface)

#define DeclareMethod(object, interface, method)\
  OBJECT_METHOD_DECLARATION(object, interface, method);

#define Method(object, interface, method)\
  OBJECT_METHOD_DECLARATION(object, interface, method) {\
    object self = (object)__data;
#define EndMethod }

#define InterfaceObjectInitializerName(interface, object)\
  _##object##interface
#define InterfaceObjectConstructorName(interface, object)\
  object##interface

#define DeclareConstructor(interface)\
  INTERFACE_OBJECT_INITIALIZER_DECLARATION(interface, OBJECT)\
  INTERFACE_OBJECT_CONSTRUCTOR_DECLARATION(interface, OBJECT)

#define DefineConstructor(interface)\
  INTERFACE_OBJECT_INITIALIZER_DEFINITION(interface, OBJECT)\
  INTERFACE_OBJECT_CONSTRUCTOR_DEFINITION(interface, OBJECT)

#define MethodDispatchFunction(interface, method, name)\
  METHOD_DISPATCH_FUNCTION_DECLARATION(interface, method, name)

#define MethodDispatchFunctionAlias(interface, method, name)\
  MethodDispatchFunction(interface, method, name) {\
    return interface##method(object METHOD_ARGUMENT_NAMES(interface, method));\
  }
#define DeclareCompositeConstructor(interface)\
  INTERFACE_COMPOSITE_INITIALIZER_DECLARATION(OBJECT, interface)\
  INTERFACE_COMPOSITE_CONSTRUCTOR_DECLARATION(OBJECT, interface)

#define DefineCompositeConstructor(interface)\
  INTERFACE_COMPOSITE_INITIALIZER_DEFINITION(OBJECT, interface)\
  INTERFACE_COMPOSITE_CONSTRUCTOR_DEFINITION(OBJECT, interface)

#endif
