#ifndef OPAQUE_H
#define OPAQUE_H

#include <stdint.h>
#include <stddef.h>
#include "Common.h"

/* Union for most of the useful data types.
 * For optimization purposes, keep size under sizeof(void*) which hopefully is
 * the CPU register size
 */
typedef union{
    uint8_t     Val_uint8_t;
    uint16_t    Val_uint16_t;
    uint32_t    Val_uint32_t;
    uint64_t    Val_uint64_t;
    int8_t      Val_int8_t;
    int16_t     Val_int16_t;
    int32_t     Val_int32_t;
    int64_t     Val_int64_t;
    size_t      Val_size_t;
    int         Val_int;
    float       Val_float;
    double      Val_double;

    intptr_t    Val_intptr_t;
    uintptr_t   Val_uintptr_t;

    void*       Val_pointer;
}OPAQUE_DATA;

/* Encapsulate static data into the appropriate OPAQUE_DATA field */
#define GENERIC_DATA(Type, Data) \
(OPAQUE_DATA){ .GLUE(Val_, Type) = Data}

/* Struct for generic memory manipulation */
TYPE_STRUCT(OPAQUE_MEMORY){
    size_t  Size;
    // If TRUE, Data contains a pointer that can be freed
    BOOLEAN Allocated;
    // If != NULL, contains a pointer to an alocated arena of size `Size`
    void*   Data;
};

#define CAST_MEMORY_AS(Mem, Type) ((Type*)((Mem)->Data))

/* Encapsulate static data into an OPAQUE_MEMORY struct */
#define CLOAK_MEMORY(Size, Alloc, Data) \
((OPAQUE_MEMORY){Size, Alloc, Data})

#define CLOAK_STRUCT(StructName, Data, Alloc) \
((OPAQUE_MEMORY){siezof(StructName), Alloc, Data})

#define CLOAK_LIST(Size, Alloc, ...) \
{Size, FALSE, (uint8_t[]){__VA_ARGS__} }

#define ITERATE_INDEXED_MEMORY(Mem, Type, Var, Ind)     \
for ((Ind = 0, Var = (Type*)((Mem)->Data));             \
      Ind != (Mem)->Size;                       \
      Var += sizeof(Type))

#define ITERATE_MEMORY(Mem, Type, Var)                                      \
for (Var = (Type*)((Mem)->Data); Var != (Type*)((Mem)->Data) + (Mem)->Size; \
     Var += sizeof(Type))

/* Setup new memory for the provided `Memory`.
 * Allocates `Size` bytes from generic memory
 */
void SetupOpaqueMemory(OPAQUE_MEMORY* Opaque, size_t Size);

/* Allocate a new OPAQUE_MEMORY and set it up */
OPAQUE_MEMORY* AllocateOpaqueMemory(size_t Size);

/* Free allocated data if (Allocated == True) */
void ClearOpaqueMemory(OPAQUE_MEMORY* Opaque);

/* Clear Opaque structure and release its' memory */
void FreeOpaqueMemory(OPAQUE_MEMORY* Opaque);

void ResizeOpaqueMemory(OPAQUE_MEMORY* Memory, size_t Length);

/* Duplicate the memory in `Base` and wrap it with an Opaque memory */
OPAQUE_MEMORY DuplicateIntoOpaqueMemory_2(const void* Base, size_t Size);

/* Appropriately resize Memory and copy into into it */
// void DuplicateIntoOpaqueMemory_3(OPAQUE_MEMORY* Memory, const void* Base, size_t Size);

#define DuplicateIntoOpaqueMemory(...) \
        GEN_OVERLOAD(DuplicateIntoOpaqueMemory, __VA_ARGS__)(__VA_ARGS__)



void CopyRawMemory_3(OPAQUE_MEMORY* Destination, const void* Base, size_t Size);
/* Copy `Size` bytes of `Base` Into the memory held by `Destination` with
 * appropriate `Offset`
 * A negative offset implies from the end of `Destination`
 */
void CopyRawMemory_4(OPAQUE_MEMORY* Destination, const void* Base, int Offset,
                     size_t Size);
#define CopyRawMemory(...) GEN_OVERLOAD(CopyRawMemory, __VA_ARGS__)(__VA_ARGS__)

void AppendRawMemory(OPAQUE_MEMORY* Destination, const void* Base, size_t Size);

void AppendOpaqueMemory_2(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source);
void AppendOpaqueMemory_3(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source, \
                          size_t Ammount);

#define AppendMemory(...) GEN_OVERLOAD(AppendMemory, __VA_ARGS__)(__VA_ARGS__)

void CopyOpaqueMemory_4(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source,
                        int Offset, size_t Ammount);
void CopyOpaqueMemory_3(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source,
                        size_t Ammount);
void CopyOpaqueMemory_2(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source);

#define CopyOpaqueMemory(...) \
        GEN_OVERLOAD(CopyOpaqueMemory, __VA_ARGS__)(__VA_ARGS__)


#ifdef ENABLE_SANITY_CHECKS

/* Assert Opaque Memory is sane */
void AssertSaneOpaqueMemory(OPAQUE_MEMORY* Opaque);

#endif

#endif /* COMMON_H */