#ifndef BASIC_LIST_H
#define BASIC_LIST_H

#include "Opaque.h"

/* For NO_DATA_ELEMENT to be compatible with Primitive and Memory, Next
 * must always be the first element
 */
TYPE_STRUCT(NO_DATA_ELEMENT) {
    NO_DATA_ELEMENT* Next;
};

// Primitive Data element
TYPE_STRUCT(PRIMITIVE_DATA_ELEMENT){
    PRIMITIVE_DATA_ELEMENT* Next;
    OPAQUE_DATA             Data;
};

// Memory Based Data element
TYPE_STRUCT(MEMORY_DATA_ELEMENT){
    MEMORY_DATA_ELEMENT*    Next;
    OPAQUE_MEMORY           Memory;
};

#ifdef ENABLE_SANITY_CHECKS
typedef enum{
    NoDataType,
    PrimitiveDataType,
    MemoryDataType
}LIST_DATA_TYPE;
#endif

TYPE_STRUCT(LIST){
    #ifdef ENABLE_SANITY_CHECKS
    LIST_DATA_TYPE InsertedTypes;
    #endif
    void* Head;
    void* Tail;
    size_t Length;
};

#define ITERATE_PRIMITIVE_DATA_TYPE(List, Type, Var)            \
for (PRIMITIVE_DATA_ELEMENT* DataElement = ((List)->Head);      \
        (DataElement != NULL) &&                                \
        ((Var) = ((DataElement)->Data.GLUE(Val_, Type)), 1);    \
    (DataElement) = (DataElement->Next))

#define ITERATE_OPAQUE_DATA_TYPE(List, Var)            \
for (PRIMITIVE_DATA_ELEMENT* DataElement = ((List)->Head);      \
        (DataElement != NULL) &&                                \
        ((Var) = ((DataElement)->Data), 1);    \
    (DataElement) = (DataElement->Next))

#define ITERATE_MEMORY_TYPE(List, Var)                  \
for (MEMORY_DATA_ELEMENT* DataElement = ((List)->Head); \
        (DataElement != NULL) &&                        \
        ((Var) = ((DataElement)->Memory), 1);           \
    (DataElement) = (DataElement->Next))

/* Allocate a new list */
LIST* AllocateList(void);

/* Insert OPAQUE_DATA into `List` */
void DataListInsert(LIST* List, OPAQUE_DATA NewData);

/* Insert OPAQUE_MEMORY into `List` */
void MemoryListInsert(LIST* List, OPAQUE_MEMORY NewMemory);

/* Allocate and recover list from provided memory
 * `ElementSize` is the size of each list element to be recovered
 */
LIST* DeSerializeDataList(OPAQUE_MEMORY* Memory, size_t ElementSize);

/* Serialize `List` into an array of bytes
 * `ElementSize` is the size of each element to be serialized
 *    it tells us what bytes from OPAQUE_DATA to use
 */
#define SerializeDataList(...) \
        GEN_OVERLOAD(SerializeDataList, __VA_ARGS__)(__VA_ARGS__)

OPAQUE_MEMORY* SerializeDataList_2(LIST* List, size_t ElementSize);
OPAQUE_MEMORY* SerializeDataList_1(LIST* List);

/* Serialize `List` into an array of bytes that can be deserialized with
 *  `DeSerializeMemoryList`
 * Format: [ Element 1 Size | Element 1 Data | Element 2 Size .. ]
 */
OPAQUE_MEMORY* SerializeMemoryList(LIST* List);

/* Serialize `List` into an array of bytes that can't be deserialized with
 *  `DeSerializeMemoryList`
 * Format: [ Element 1 Data | Element 2 Data | Element 3 Data .. ]
 */
OPAQUE_MEMORY* SerializeMemoryListElements(LIST* List);

/* Allocate and recover list from provided memory */
LIST* DeSerializeMemoryList(OPAQUE_MEMORY* Memory);

/* Clear all elements in Data List */
void ClearDataList(LIST* List);

/* Clear all elements in Memory List */
void ClearMemoryList(LIST* List);

void FreeDataList(LIST* List);
void FreeMemoryList(LIST* List);


#ifdef ENABLE_SANITY_CHECKS

// Validate our usage of NO_DATA_ELEMENT (Next is at the same offset everywhere)
// static_assert(offsetof(NO_DATA_ELEMENT, Next) ==
//               offsetof(PRIMITIVE_DATA_ELEMENT, Next));

// static_assert(offsetof(MEMORY_DATA_ELEMENT, Next) ==
//               offsetof(PRIMITIVE_DATA_ELEMENT, Next));

// // Validate we arekeeping OPAQUE_DATA under register size
// static_assert(sizeof(OPAQUE_DATA) <= sizeof(void*));

// Validate that we aren't mixing types
void ValidateInsertion(LIST* List, LIST_DATA_TYPE DataType);

// Validate List is sane
void AssertSaneList(LIST* List);

// Validate List is a Memory List and its' sane
void AssertSaneMemoryList(LIST* List);

// Validate Data is a Memory List and its' sane
void AssertSaneDataList(LIST* List);

#endif /* ENABLE_SANITY_CHECKS */

#endif /* BASIC_LIST_H */