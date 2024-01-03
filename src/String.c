#include "String.h"
#include "Common.h"

#ifdef ENABLE_SANITY_CHECKS

void AssertSaneString(STRING* Str) {
    Assert(Str != NULL);
    Assert(Str->Memory.Size >= Str->Length);
    Assert(strlen(Str->Memory.Data) == Str->Length);
}

#endif

// static const char EmptyString;

STRING* GenEmptyString_0(void) {
    return GenEmptyString_1(0);
    // ALLOC_STRUCT(STRING, NewString);

    // NewString->Length = 0;
    // NewString->Memory.Data = NULL;
    // NewString->Memory.Allocated = FALSE;

    // return NewString;
}

STRING* GenEmptyString_1(size_t InitialSize) {
    ALLOC_STRUCT(STRING, NewString);

    SetupOpaqueMemory(&(NewString->Memory), InitialSize + 1);
    *( (char*)(NewString->Memory.Data) ) = '\0';
    NewString->Length = 0;

    return NewString;
}

STRING* CharArrToString_1(const char* Arr) {
    return CharArrToString_2(Arr, Strlen(Arr));
}

STRING* CharArrToString_2(const char* Arr, size_t KnownSize) {
    ALLOC_STRUCT(STRING, NewString);

    NewString->Length = KnownSize;
    NewString->Memory = DuplicateIntoOpaqueMemory((void*)Arr, NewString->Length + 1);

    return NewString;
}

STRING* MemoryToString(OPAQUE_MEMORY* Memory) {
    ALLOC_STRUCT(STRING, NewString);

    NewString->Length = Strlen(Memory->Data);
    NewString->Memory = *Memory;

    return NewString;
}

// Allocate exact memory amount for the string
// static void OptimizeString(STRING* String) {
//     if (String->Length + 1 != String->Memory.Size) {
//         ResizeOpaqueMemory(&(String->Memory), String->Length + 1);
//         // Always add \0 just in case ;)
//         CAST_MEMORY_AS(&(String->Memory), char)[String->Length] = '\0';
//     }
// }

void StringConcatString(STRING* Destination, STRING* Source) {
    size_t DstLen = Destination->Length;
    size_t SrcLen = Source->Length;

    CopyOpaqueMemory(&(Destination->Memory), &(Source->Memory), DstLen, SrcLen + 1);

    Destination->Length = DstLen + SrcLen;
}

void StringConcatCharArr(STRING* Receiver, const char* Source) {
    SANITY_CHECK( Assert(Source != NULL) );
    SANITY_CHECK( AssertSaneString(Receiver) );

    size_t SrcLen = strlen(Source);
    CopyRawMemory(&(Receiver->Memory), Source, Receiver->Length, SrcLen + 1);
    Receiver->Length += SrcLen;
}

void SetString(STRING* Receiver, const char* Source) {
    size_t SrcLen = Strlen(Source);
    ClearOpaqueMemory(&(Receiver->Memory));
    Receiver->Memory = DuplicateIntoOpaqueMemory(Source, SrcLen + 1);
}


const char* StringToCStr(STRING* String) {
    return (const char*)(String->Memory.Data);
}

#include <stdarg.h>

/* IDEA
 * If there is the need to format big pieces of code, it might be a good idea to
 *  simply break the format string into several pieces, making sure there is no
 *  '%' in 3/4 (need to double check) characters preceding the cut so we don't
 *  mess up the format
 */
static STRING* _FormatSizedString(size_t ExpectedSize, const char* Format,
                                  va_list ArgList) {
    size_t BufSize = ExpectedSize + 1;
    char* Buffer = (char*)Malloc(BufSize);
    size_t WrittenChars;

    size_t PreviouslyWrittenChars = 0;

    while(TRUE){
        WrittenChars = Vsnprintf(Buffer, BufSize, Format, ArgList);
        // A bigger buffer didn't change written chars, we can stop here
        if (WrittenChars == PreviouslyWrittenChars) {
            break;
        }
        // If we stopped before the limit, we know it wasn't a problem
        if (WrittenChars < BufSize) {
            break;
        }

        // We need more space!
        PreviouslyWrittenChars = WrittenChars;
        BufSize += ExpectedSize;
        Buffer = Realloc(Buffer, BufSize);
    }

    // Pack into String
    STRING* Final = CharArrToString(Buffer, WrittenChars);

    Free(Buffer);

    return Final;
}

STRING* FormatSizedString(size_t ExpectedSize, const char* Format, ...) {
    va_list ArgList;
    STRING* Result;
  
    va_start(ArgList,Format);
    Result = _FormatSizedString(ExpectedSize, Format, ArgList);
    va_end(ArgList);

    return Result;
}

STRING* FormatString(const char* Format, ...) {
    va_list ArgList;
    STRING* Result;
  
    va_start(ArgList,Format);
    Result = _FormatSizedString(AVERAGE_STRING_SIZE_HEURISTIC, Format, ArgList);
    va_end(ArgList);

    return Result;
}

void StringFormatAppend(STRING* Str, const char* Format, ...) {
    va_list ArgList;
    STRING* Result;
  
    va_start(ArgList,Format);
    Result = _FormatSizedString(AVERAGE_STRING_SIZE_HEURISTIC, Format, ArgList);
    va_end(ArgList);

    StringConcatString(Str, Result);

    FreeString(Result);
}

void ClearString(STRING* Str) {
    ClearOpaqueMemory(&(Str->Memory));
    Str->Length = 0;
}

void FreeString(STRING* Str) {
    ClearOpaqueMemory(&(Str->Memory));
    Free(Str);
}

