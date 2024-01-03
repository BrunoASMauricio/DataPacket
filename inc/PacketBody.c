
// Include from a single source file
// Must have the header importing packet_header.hpp present

#include "Common.h"
#include "DataPacket.h"

// Base structure definition
#define PACKET_FIELD(Type, Field)                                       \
NewPacketFieldMetadata(new_meta->Fields, false, STR(Type),           \
                          STR(Field), offsetof(PACKET_NAME, Field),     \
                          sizeof(((PACKET_NAME*)0)->Field));

#define PACKET_FIELD_DYNAMIC(Field)                                     \
NewPacketFieldMetadata(new_meta->Fields, true, "OPAQUE_MEMORY",      \
                          STR(Field), offsetof(PACKET_NAME, Field),     \
                          sizeof(((PACKET_NAME*)0)->Field));

#define PACKET_START()                                                  \
{                                                                       \
PACKET_METADATA* new_meta = NewPacketMetadata(STR_1(PACKET_NAME),     \
                                                sizeof(PACKET_NAME));   \
PACKET_FIELD(BASE_PACKET, base);

#define PACKET_FINALIZE() \
    DataListInsert(&PacketDefinitionsList, GENERIC_DATA(pointer, new_meta)); \
    FinalizePacket(new_meta);                                                \
}

// Process the .def file
#include PACKET_DEF_FILE

#ifndef PACKET_NAME
#error Packet header called without PACKET_NAME defined
#endif

#undef PACKET_NAME
#undef PACKET_START
#undef PACKET_FIELD_DYNAMIC
#undef PACKET_FIELD
#undef PACKET_FINALIZE


