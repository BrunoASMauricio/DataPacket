#include "Opaque.h"

#ifdef ENABLE_SANITY_CHECKS

void AssertSaneOpaqueMemory(OPAQUE_MEMORY* Opaque) {
    Assert(Opaque != NULL);
    if (Opaque->Allocated == TRUE) {
        Assert(Opaque->Data != NULL);
        Assert(Opaque->Size > 0);
    }
}

#endif

void SetupOpaqueMemory(OPAQUE_MEMORY* Opaque, size_t Size) {
    SANITY_CHECK( Assert(Opaque != NULL) );

    Opaque->Size = Size;
    if (Size > 0) {
        Opaque->Data = Malloc(Opaque->Size);

        SANITY_CHECK( Assert(Opaque->Data != NULL) );

        if (Opaque->Data != NULL) {
            Opaque->Allocated = TRUE;
            return;
        }
    }
    // error fall through
    Opaque->Data = NULL;
    Opaque->Allocated = FALSE;
}

void CopyRawMemory_4(OPAQUE_MEMORY* Destination, const void* Base, int Offset,
                     size_t Size) {
    SANITY_CHECK( AssertSaneOpaqueMemory(Destination) );
    SANITY_CHECK( Assert(Base != NULL) );

    // We cant resize Destination and copy into it because we run the risc of
    // undefined behavior in case `Base` is the address stored in Destination
    // (i.e. Copy/Append(X, X))

    uint8_t*    NewMemory;
    uint8_t*    NewMemoryIndex;
    size_t      FirstSliceSize;
    size_t      NewSize;

    // Offset Destination backwards
    if (Offset < 0) {
        SANITY_CHECK( Assert((size_t)(-1 * Offset) >= Destination->Size) );

        NewSize         = Destination->Size + Offset + Size;
        FirstSliceSize  = Destination->Size + Offset;
    } else {
        NewSize         = Offset + Size;
        FirstSliceSize  = Offset;
    }

    NewMemory = (uint8_t*)Malloc(NewSize);
    NewMemoryIndex = NewMemory;

    Memcpy(NewMemoryIndex, Destination->Data, FirstSliceSize);
    NewMemoryIndex += FirstSliceSize;
    Memcpy(NewMemoryIndex, Base, Size);

    SANITY_CHECK( Assert((size_t)(NewMemoryIndex - NewMemory) + Size == NewSize) );

    ClearOpaqueMemory(Destination);
    Destination->Data = NewMemory;
    Destination->Allocated = TRUE;
    Destination->Size = NewSize;
}

void CopyRawMemory_3(OPAQUE_MEMORY* Destination, const void* Base, size_t Size) {
    CopyRawMemory_4(Destination, Base, 0, Size);
}

void AppendRawMemory(OPAQUE_MEMORY* Destination, const void* Base, size_t Size) {
    CopyRawMemory_4(Destination, Base, Destination->Size, Size);
}

void CopyOpaqueMemory_4(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source, \
                        int Offset, size_t Ammount) {
    SANITY_CHECK( AssertSaneOpaqueMemory(Source) );
    SANITY_CHECK( Assert(Ammount <= Source->Size) );

    CopyRawMemory_4(Destination, Source->Data, Offset, Ammount);
}

void CopyOpaqueMemory_3(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source, \
                        size_t Ammount) {
    CopyOpaqueMemory_4(Destination, Source, 0, Ammount);
}

void CopyOpaqueMemory_2(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source) {
    CopyOpaqueMemory_4(Destination, Source, 0, Source->Size);
}

void AppendOpaqueMemory_2(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source) {
    CopyOpaqueMemory_4(Destination, Source, Destination->Size, Source->Size);
}

void AppendOpaqueMemory_3(OPAQUE_MEMORY* Destination, OPAQUE_MEMORY* Source, \
                          size_t Ammount) {
    CopyOpaqueMemory_4(Destination, Source, Destination->Size, Ammount);
}

OPAQUE_MEMORY DuplicateIntoOpaqueMemory_2(const void* Base, size_t Size) {
    SANITY_CHECK( Assert(Base != NULL) );

    OPAQUE_MEMORY Opaque;
    Opaque.Size = Size;
    Opaque.Data = DuplicateGenericMemory(Base, Size);
    if (Opaque.Data != NULL) {
        Opaque.Allocated = TRUE;
    } else {
        Opaque.Allocated = FALSE;
    }
    return Opaque;
}

// void DuplicateIntoOpaqueMemory_3(OPAQUE_MEMORY* Memory, const void* Base, size_t Size) {
//     SANITY_CHECK( Assert(Base != NULL) );
//     SANITY_CHECK( AssertSaneOpaqueMemory(Memory) );

//     if (Size != Memory->Size || Memory->Allocated == FALSE) {
//         FreeOpaqueMemory(Memory);
//         Memory = DuplicateIntoOpaqueMemory_2(Base, Size);
//     } else {
//         Memcpy(Memory->Data, Base, Size);
//     }
// }


OPAQUE_MEMORY* AllocateOpaqueMemory(size_t Size) {
    ALLOC_STRUCT(OPAQUE_MEMORY, Opaque);
    Opaque->Allocated = FALSE;
    SetupOpaqueMemory(Opaque, Size);
    return Opaque;
}

void ClearOpaqueMemory(OPAQUE_MEMORY* Opaque) {
    SANITY_CHECK( Assert(Opaque != NULL) );

    if (Opaque->Allocated == TRUE) {
        Free(Opaque->Data);
    }
    Opaque->Allocated = FALSE;
}

void ResizeOpaqueMemory(OPAQUE_MEMORY* Memory, size_t NewSize) {
    if (NewSize == Memory->Size) {
        return;
    }

    printf("%p\n", Memory->Data);

    void* New = Malloc(NewSize);
    Memcpy(New, Memory->Data, Memory->Size);
    Free(Memory->Data);
    Memory->Data = New;
    Memory->Size = NewSize;

    printf("%p\n", Memory->Data);

    // #ifdef ENABLE_SANITY_CHECKS
    //     void* Ret = Realloc(Memory->Data, Memory->Size);
    //     Assert(Ret != NULL);
    //     Memory->Data = Ret;
    // #else
    //     Memory->Data = Realloc(Memory->Data, Memory->Size);
    // #endif
}


void FreeOpaqueMemory(OPAQUE_MEMORY* Opaque) {
    SANITY_CHECK( Assert(Opaque != NULL) );

    ClearOpaqueMemory(Opaque);
    Free(Opaque);
}
