#ifndef DATA_PACKET_HPP
#define DATA_PACKET_HPP

// #include "DoubleLinkList.hpp"
#include "Common.h"
#include "Opaque.h"

// #include <vector>
#include <stdint.h>
#include <stdbool.h>

#include "BasicList.h"
#include "String.h"

typedef uint8_t PACKET_METADATA_flags_t;
typedef uint8_t PACKET_ID;

//      packet metadata flag bits
// necessary for endianness fixing
#define MUlTI_BYTE_FIELDS   0
// Fields with dynamic sizes
#define DYNAMIC_FIELDS      1

TYPE_STRUCT(PacketFieldMetadata){
    bool        IsDynamic;
    const char* FieldType;
    const char* FieldName;
    size_t      FieldOffset;
    size_t      FieldSize;
};

enum endianness_t : uint8_t {
    BigEndian,
    LittleEndian
};
typedef enum endianness_t endianness_t;

/*
BASE_PACKET elements should all be 1 byte, so packets with 1 byte elements don't
become multi-byte by default
*/
TYPE_STRUCT(BASE_PACKET){
    struct flags{
        endianness_t endianness : 1;
    }flags;
    PACKET_ID     PacketId;
    
};

TYPE_STRUCT(PACKET_METADATA){
    PACKET_ID                         PacketId;
    const char*                         PacketName;
    size_t                              PacketSize;
    PACKET_METADATA_flags_t             PacketFlags;
    // vector<PacketFieldMetadata>*   Fields;
    LIST*                               Fields;
};

extern LIST PacketDefinitionsList;

// Retrieve 1 bit endianness of the running system
endianness_t GetEndianness(void);

PACKET_METADATA* GetPacketMetadata(PACKET_ID PacketId);

PACKET_ID GetPacketId(const char* name);

void NewPacketFieldMetadata(LIST* FieldList, bool _IsDynamic,
                                const char* _FieldType, const char* _FieldName,
                                size_t _FieldOffset, size_t _FieldSize);

//              General Packet operations
/* Allocates packet memory for the provided id
 * Returned packet must be released by the user via PacketDeallocate
 */
BASE_PACKET* PacketAllocate(PACKET_ID PacketId);

/* Deallocates packet memory. Non-NULL dynamic Fields are freed
 */
void PacketDeallocate(void* _packet);

/* Allocates memory and fills it with packet representation
 * Releasing this memory (with free) is the users responsibility
 */
OPAQUE_MEMORY PacketSerialize(void* _packet);

/* Allocates and fills packet from provided memory
 * Returned packet must be released by the user via PacketDeallocate
 */
BASE_PACKET* PacketDeserialize(OPAQUE_MEMORY mem);


// //              Opener and closer for packet usage
void FinalizePackets(void);
void ReleasePackets(void);

// //              Packet definitions
STRING* PacketToString(PACKET_METADATA* PacketMeta);
STRING* PacketsToString(void);
// string packet_to_string(void* packet);
// string PacketsToString();

// //              Packet Setup helpers
PACKET_METADATA* NewPacketMetadata(const char* _PacketName,
                                    size_t _PacketSize);

void FinalizePacket(PACKET_METADATA* meta);

#endif