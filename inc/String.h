#ifndef STRING_H
#define STRING_H

#include "Common.h"
#include "Opaque.h"

#define AVERAGE_STRING_SIZE_HEURISTIC 100

TYPE_STRUCT(STRING) {
    // NULL terminated string
    OPAQUE_MEMORY Memory;
    // strlen cache
    size_t        Length;
};

#ifdef ENABLE_SANITY_CHECKS

void AssertSaneString(STRING* Str);

#endif

const char* StringToCStr(STRING* Receiver);

STRING* GenEmptyString_0(void);

STRING* GenEmptyString_1(size_t InitialSize);

#define GenEmptyString(...) GEN_OVERLOAD(GenEmptyString, __VA_ARGS__)(__VA_ARGS__)

STRING* CharArrToString_1(const char* Arr);

STRING* CharArrToString_2(const char* Arr, size_t KnownSize);

#define CharArrToString(...) GEN_OVERLOAD(CharArrToString, __VA_ARGS__)(__VA_ARGS__)

STRING* MemoryToString(OPAQUE_MEMORY* Memory);

void StringConcatString(STRING* Receiver, STRING* Source);

void StringConcatCharArr(STRING* Receiver, const char* Source);

void SetString(STRING* Receiver, const char* Source);

STRING* FormatSizedString(size_t ExpectedSize, const char* Format, ...);

STRING* FormatString(const char* Format, ...);

void StringFormatAppend(STRING* Str, const char* Format, ...);

void ClearString(STRING* Str);
void FreeString(STRING* Str);

#endif