#include <stdio.h>

#include "tests/Basic.h"
#include "tests/MultiByte.h"
#include "tests/Dynamic.h"

#include "tests/ExpectedPacketMetadata.inc"

#include "DataPacket.h"
#include "String.h"

static void ValidateString(STRING* String, const char* Expected) {
    assert( 0 == strcmp(StringToCStr(String), Expected) );
}

static void StringTest(void) {

    STRING* String1 = CharArrToString("Hello Dear");

    ValidateString(String1, "Hello Dear");

    StringConcatString(String1, String1);

    ValidateString(String1, "Hello DearHello Dear");

    SetString(String1, "Nothing At all!");

    ValidateString(String1, "Nothing At all!");

    int SomeInt = 43;
    STRING* String2 = FormatString("%d --> %s %04x cool\n", SomeInt,
                                   StringToCStr(String1), 0x1234);

    ValidateString(String2, "43 --> Nothing At all! 1234 cool\n");

    FreeString(String2);
    FreeString(String1);
}

int main(void) {
    StringTest();

    BasicPacketSetup();
    MultiBytePacketSetup();
    DynamicPacketSetup();

    FinalizePackets();

    BasicTest();
    multibyte_test();
    dynamic_test();

    STRING* PacketMetas = PacketsToString();
    ValidateString(PacketMetas, ExpectedPacketMeta);
    FreeString(PacketMetas);

    ReleasePackets();
}