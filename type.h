#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>

typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

/* Boolean */
typedef s4 b4;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

typedef float f4;
typedef double f8;

typedef void *Any;

/* Unsigned pointer-sized integer */
typedef size_t uptr;

typedef u1 Byte;
typedef u1 *Bytes;

typedef u1 *MString;
typedef const MString String;

#endif
