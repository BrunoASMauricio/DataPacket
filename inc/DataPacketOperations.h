#ifndef DATA_PACKET_OPERATIONS_HPP
#define DATA_PACKET_OPERATIONS_HPP

#include "DataPacket.h"

typedef BASE_PACKET*   (*packet_allocator_f)(PACKET_ID);
typedef void            (*packet_deallocator_f)(BASE_PACKET*);

typedef OPAQUE_MEMORY   (*PacketSerialize_f)(void*);
typedef BASE_PACKET*   (*PacketDeserialize_f)(PACKET_ID, OPAQUE_MEMORY);

struct PACKET_OPS{
    packet_allocator_f      allocate;
    packet_deallocator_f    deallocate;
    PacketSerialize_f      serialize;
    PacketDeserialize_f    deserialize;
};
typedef struct PACKET_OPS PACKET_OPS;

/* If the packet reports different endianess from current system, fix MBY Fields */
void fix_packet_endianness(PACKET_METADATA* meta, void* _packet);

// /* Allocate and 0 initialize a packet: BSC */
BASE_PACKET* BasicAllocate(PACKET_ID PacketId);

// /* Wrapper for free: BSC & MBY */
void BasicDeallocate(BASE_PACKET* packet);

/* Wrapper for memcpy and memory initializer: BSC & MBY */
OPAQUE_MEMORY BasicSerialize(void* _packet);

/* Wrapper for memcpy and packet initializer: BSC */
BASE_PACKET* BasicDeserialize(PACKET_ID PacketId, OPAQUE_MEMORY mem);

/* Allocate, 0 initialize and setup endianess flag for a packet: MBY */
BASE_PACKET* MultiByteAllocate(PACKET_ID PacketId);

/* Wrapper for memcpy, packet initializer and endianess fix: MBY */
BASE_PACKET* MultiByteDeserialize(PACKET_ID PacketId, OPAQUE_MEMORY mem);

/* Wrapper for free, that also deallocates non-NULL DYN Fields: DYN */
void AdvancedDeallocate(BASE_PACKET* packet);

/* Serialize Fields, properly taking care of DYN serialization: DYN */
OPAQUE_MEMORY AdvancedSerialize(void* _packet);

/* Deserialize Fields, properly taking care of DYN serialization: DYN */
BASE_PACKET* AdvancedDeserialize(PACKET_ID PacketId, OPAQUE_MEMORY mem);


#endif