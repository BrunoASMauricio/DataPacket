
#include "DataPacket.h"

// Include from header file

// Base structure definition
#define PACKET_START() \
struct PACKET_NAME {                       \
BASE_PACKET base;

#define PACKET_FIELD(type, field) \
type field;

#define PACKET_FIELD_DYNAMIC(field) \
OPAQUE_MEMORY field;

#define PACKET_FINALIZE() \
}DONT_PAD;\
typedef struct PACKET_NAME PACKET_NAME;

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


