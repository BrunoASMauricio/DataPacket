#include <stdlib.h>

#include "Common.h"
#include "BasicList.h"

#ifdef ENABLE_SANITY_CHECKS

// Validate that we aren't mixing types
void ValidateInsertion(LIST* List, LIST_DATA_TYPE DataType) {
    if (List->InsertedTypes == NoDataType) {
        List->InsertedTypes = DataType;
    } else {
        Assert(List->InsertedTypes == DataType);
    }
}

void AssertSaneList(LIST* List) {
    Assert(List != NULL);

    size_t RealLength = 0;
    NO_DATA_ELEMENT* ExpectedTail = NULL;

    for (NO_DATA_ELEMENT* ListElement = ((List)->Head);
        ListElement != NULL;
        ListElement = ListElement->Next) {
        RealLength += 1;
        ExpectedTail = ListElement;
    }

    Assert(RealLength   == List->Length);
    Assert(ExpectedTail == List->Tail);
}

void AssertSaneMemoryList(LIST* List) {
    AssertSaneList(List);
    switch (List->InsertedTypes) {
        case MemoryDataType:
        case NoDataType:
            return;
        default:
    }
    Assert("Invalid type for memory list" == 0);
}

void AssertSaneDataList(LIST* List) {
    AssertSaneList(List);
    switch (List->InsertedTypes) {
        case PrimitiveDataType:
        case NoDataType:
            return;
        default:
    }
    Assert("Invalid type for data list" == 0);
}

#endif

LIST* AllocateList(void) {
    ALLOC_STRUCT(LIST, NewList);
    NewList->Head   = NULL;
    NewList->Tail   = NULL;
    NewList->Length = 0;

    #ifdef ENABLE_SANITY_CHECKS
    NewList->InsertedTypes = NoDataType;
    #endif

    return NewList;
}

static void AddListElement(LIST* List, void* _NewLink) {
    NO_DATA_ELEMENT* NewLink = _NewLink;
    NO_DATA_ELEMENT* Tail = List->Tail;
    if (List->Head == NULL) {
        NewLink->Next   = NULL;
        List->Head      = NewLink;
        List->Tail      = NewLink;
    } else {
        Tail->Next = NewLink;
        List->Tail = NewLink;
    }
    List->Length += 1;
}

void MemoryListInsert(LIST* List, OPAQUE_MEMORY NewMemory) {
    SANITY_CHECK( AssertSaneMemoryList(List) );
    SANITY_CHECK( ValidateInsertion(List, MemoryDataType) );

    ALLOC_STRUCT(MEMORY_DATA_ELEMENT, NewLink);
    NewLink->Memory = NewMemory;
    NewLink->Next = NULL;

    AddListElement(List, NewLink);
}

void DataListInsert(LIST* List, OPAQUE_DATA NewData) {
    SANITY_CHECK( AssertSaneDataList(List) );
    SANITY_CHECK( ValidateInsertion(List, PrimitiveDataType) );

    ALLOC_STRUCT(PRIMITIVE_DATA_ELEMENT, NewLink);
    NewLink->Data = NewData;
    NewLink->Next = NULL;

    AddListElement(List, NewLink);
}

OPAQUE_MEMORY* SerializeDataList_2(LIST* List, size_t ElementSize) {
    SANITY_CHECK( AssertSaneDataList(List) );

    OPAQUE_MEMORY* Total = AllocateOpaqueMemory(ElementSize * List->Length);
    uint8_t* MemoryIndex;
    MemoryIndex = (uint8_t*)Total->Data;
    uintptr_t Element;

    ITERATE_PRIMITIVE_DATA_TYPE(List, uintptr_t, Element) {
        int Start, Direction;
        if (BYTE_ORDER == LITTLE_ENDIAN) {
            Start = 0;
            Direction = 1;
        } else {
            Start = sizeof(OPAQUE_DATA) - 1;
            Direction = -1;
        }
        // 0 -> ElementSize
        for(int Ind = Start; (Ind < (int)ElementSize && Ind >= 0); Ind += Direction) {
            MemoryIndex[Ind] = Element & 0xFF;
            Element = Element >> 8;
        }
        MemoryIndex += ElementSize;
    }

    return Total;
}

OPAQUE_MEMORY* SerializeDataList_1(LIST* List) {
    SANITY_CHECK( AssertSaneDataList(List) );

    OPAQUE_MEMORY* Total = AllocateOpaqueMemory(sizeof(OPAQUE_DATA) * List->Length);
    uint8_t* MemoryIndex;
    MemoryIndex = (uint8_t*)Total->Data;
    uintptr_t Element;

    ITERATE_PRIMITIVE_DATA_TYPE(List, uintptr_t, Element) {
        Memcpy(MemoryIndex, &Element, sizeof(Element));
        // 0 -> ElementSize
        *((uintptr_t*)MemoryIndex) = Element;
        MemoryIndex += sizeof(Element);
    }

    return Total;
}

LIST* DeSerializeDataList(OPAQUE_MEMORY* Memory, size_t ElementSize) {
    SANITY_CHECK( AssertSaneOpaqueMemory(Memory) );

    uint8_t* MemoryIndex = Memory->Data;
    intptr_t Field;
    LIST* List = AllocateList();
    while (MemoryIndex < (uint8_t*)Memory->Data + Memory->Size) {
        // Assume same endianness
        Memcpy(&Field, MemoryIndex, ElementSize);
        DataListInsert(List, GENERIC_DATA(intptr_t, Field));
        MemoryIndex += ElementSize;
    }
    SANITY_CHECK(Assert(MemoryIndex == (uint8_t*)Memory->Data + Memory->Size));
    return List;
}

LIST* DeSerializeMemoryList(OPAQUE_MEMORY* Memory) {
    SANITY_CHECK( AssertSaneOpaqueMemory(Memory) );

    uint8_t* MemoryIndex = Memory->Data;
    size_t FieldSize;
    LIST* List = AllocateList();
    while (MemoryIndex < (uint8_t*)Memory->Data + Memory->Size) {
        // Assume same endianness
        Memcpy(&FieldSize, MemoryIndex, sizeof(FieldSize));
        MemoryIndex += sizeof(FieldSize);
        MemoryListInsert(List, DuplicateIntoOpaqueMemory(MemoryIndex, FieldSize));
        MemoryIndex += FieldSize;
    }

    SANITY_CHECK( Assert(MemoryIndex == (uint8_t*)Memory->Data + Memory->Size) );

    return List;
}

static size_t SerializedMemoryListSize(LIST* List) {
    SANITY_CHECK( AssertSaneMemoryList(List) );

    size_t TotalSize = 0;
    OPAQUE_MEMORY Element;

    ITERATE_MEMORY_TYPE(List, Element) {
        TotalSize += Element.Size;
    }
    return TotalSize;
}

OPAQUE_MEMORY* SerializeMemoryList(LIST* List) {
    SANITY_CHECK( AssertSaneMemoryList(List) );

    size_t DynamicSize;
    OPAQUE_MEMORY* Total;
    uint8_t* MemoryIndex;

    DynamicSize = SerializedMemoryListSize(List);
    DynamicSize += FIELD_SIZE(OPAQUE_MEMORY, Size) * List->Length;

    Total = AllocateOpaqueMemory(DynamicSize);
    MemoryIndex = (uint8_t*)Total->Data;

    OPAQUE_MEMORY Element;
    ITERATE_MEMORY_TYPE(List, Element) {
        Memcpy(MemoryIndex, &(Element.Size), sizeof(Element.Size));
        MemoryIndex += sizeof(Element.Size);
        Memcpy(MemoryIndex, Element.Data, Element.Size);
        MemoryIndex += Element.Size;
    }
    return Total;
}

OPAQUE_MEMORY* SerializeMemoryListElements(LIST* List) {
    SANITY_CHECK( AssertSaneMemoryList(List) );

    OPAQUE_MEMORY* Total = AllocateOpaqueMemory(SerializedMemoryListSize(List));

    uint8_t* MemoryIndex;
    MemoryIndex = (uint8_t*)Total->Data;

    OPAQUE_MEMORY Element;
    ITERATE_MEMORY_TYPE(List, Element) {
        Memcpy(MemoryIndex, Element.Data, Element.Size);
        MemoryIndex += Element.Size;
    }
    return Total;
}


void ClearDataList(LIST* List) {
    SANITY_CHECK( AssertSaneDataList(List) );

    PRIMITIVE_DATA_ELEMENT* Current = List->Head;
    while(Current != NULL) {
        PRIMITIVE_DATA_ELEMENT* Next = Current->Next;
        Free(Current);
        Current = Next;
    }
}

void ClearMemoryList(LIST* List) {
    SANITY_CHECK( AssertSaneMemoryList(List) );

    MEMORY_DATA_ELEMENT* Current = List->Head;
    while(Current != NULL) {
        MEMORY_DATA_ELEMENT* Next = Current->Next;
        ClearOpaqueMemory(&(Current->Memory));
        Free(Current);
        Current = Next;
    }
}

void FreeDataList(LIST* List) {
    SANITY_CHECK( AssertSaneDataList(List) );

    ClearDataList(List);
    Free(List);
}

void FreeMemoryList(LIST* List) {
    SANITY_CHECK( AssertSaneMemoryList(List) );

    ClearMemoryList(List);
    Free(List);
}
