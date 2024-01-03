#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

//                      stdlib.h wrappers
#define Malloc(BaseAddress) malloc(BaseAddress)
#define Free(BaseAddress) free(BaseAddress)
#define Assert(Condition) assert(Condition)
#define Realloc(Base, Size) realloc(Base, Size)
#define Vsnprintf(Str, Size, Format, ArgList) vsnprintf(Str, Size, Format, ArgList)

//                      string.h wrappers
#define Memcpy memcpy
#define Strcpy strcpy
#define Strlen strlen

//                      type wrappers
#ifndef BOOLEAN
typedef unsigned char BOOLEAN;
#define TRUE  ((BOOLEAN)(1==1))
#define FALSE ((BOOLEAN)(0==1))
#endif

//                      Helper macros
#define GLUE(X, Y) X ## Y
#define STR(X) #X
// once deferred macro to string conversion
#define STR_1(X) STR(X)

#define ARRAY_SIZE(Array) (sizeof(Array) / Array[0])

//          Allow parameter amount overloading without _Generic (up to 10 parameters)

/* ARG_COUNT and ARG_COUNT_HELPER help infering the suffix to use from the
 *   amount of arguments in __VA_ARGS__
 * The X and __
 */
#define _OVERLOAD_ARG_COUNT_HELPER(X, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
                                   COUNT, ...) COUNT

#define _OVERLOAD_ARG_COUNT(...) _OVERLOAD_ARG_COUNT_HELPER(__VA_OPT__(X,),  \
                                   __VA_ARGS__, _10, _9, _8, _7, _6, _5, _4, \
                                   _3, _2, _1, _0)

/* Prevent NAME from being evaluated too early by concatenating with (empty)
 *  __VA_ARGS__
 */
#define _OVERLOAD_GLUE(NAME, SUFFIX, ...) GLUE(NAME ## __VA_ARGS__, SUFFIX)

/* Setup a function to be overloadable
 * Usage example: Define FunctionName_1 FunctionName_4 that use 1 and 4
 *   parameters respectively
 * Then, define the following macro such that FunctionName can be used without _X
 *
 *  #define FunctionName(...) GEN_OVERLOAD(FunctionName, __VA_ARGS__)(__VA_ARGS__)
 *
 */
#define GEN_OVERLOAD(NAME, ...) _OVERLOAD_GLUE(NAME, _OVERLOAD_ARG_COUNT(__VA_ARGS__))


/* struct typedef which allows self-referencing without needing the struct
 * keyword in the Fields type
 */
#define TYPE_STRUCT(Name) \
typedef struct Name Name; \
struct Name

#define ALLOC_STRUCT(TYPE, VAR) \
TYPE* VAR = (TYPE*)Malloc(sizeof(TYPE))

/* Toggleable sanity checks */
#ifdef ENABLE_SANITY_CHECKS
#define SANITY_CHECK(...) __VA_ARGS__
#else
#define SANITY_CHECK(...)
#endif

//                  Bit manipulation macros
#define CHECK_BIT_AT(value, pos)    ((value >> pos) & 1U)
#define SET_BIT_AT(value, pos)      value |= 1UL << pos;

//                      Compiler wrappers
#define RUN_BEFORE_MAIN __attribute__((constructor))
#define DONT_PAD __attribute__((packed))
#define FIELD_SIZE(Struct, Field) (sizeof(((Struct*)0)->Field))

//          Common data structures, their macros and functions

/* Copy the data provided into a new generic memory location */
void* DuplicateGenericMemory(const void* Base, size_t Size);

#endif /* COMMON_H */