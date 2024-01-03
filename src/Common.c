#include "Common.h"

void* DuplicateGenericMemory(const void* Base, size_t Size) {
    SANITY_CHECK( Assert(Base != NULL) );

    void* NewAddress = Malloc(Size);

    SANITY_CHECK( Assert(NewAddress != NULL) );

    Memcpy(NewAddress, Base, Size);

    return NewAddress;
}
