#include "DataPacketOperations.h"

#include <byteswap.h>
#include <stdlib.h>
#include <string.h>

BASE_PACKET* BasicAllocate(PACKET_ID PacketId) {
    PACKET_METADATA* PacketMeta = GetPacketMetadata(PacketId);
    BASE_PACKET* new_packet = (BASE_PACKET*)calloc(1, PacketMeta->PacketSize);
    new_packet->PacketId = PacketMeta->PacketId;
    return new_packet;
}

void BasicDeallocate(BASE_PACKET* packet) {
    Free(packet);
}

OPAQUE_MEMORY BasicSerialize(void* _packet) {
    OPAQUE_MEMORY mem;
    BASE_PACKET* packet = (BASE_PACKET*)_packet;
    PACKET_METADATA* PacketMeta = GetPacketMetadata(packet->PacketId);

    mem.Size = PacketMeta->PacketSize;
    mem.Data = (BASE_PACKET*)malloc(mem.Size);
    mem.Allocated = TRUE;

    memcpy(mem.Data, packet, mem.Size);
    return mem;
}

BASE_PACKET* BasicDeserialize(PACKET_ID PacketId, OPAQUE_MEMORY mem) {
    BASE_PACKET* packet_res;
    PACKET_METADATA* PacketMeta = GetPacketMetadata(PacketId);
    packet_res = (BASE_PACKET*)malloc(PacketMeta->PacketSize);
    memcpy(packet_res, mem.Data, mem.Size);
    return packet_res;
}



static void fix_field_endianness(PacketFieldMetadata* FieldMeta, 
                                 BASE_PACKET* packet) {
    // !=0 to ignore the BASE_PACKET (which MUST only be 1 byte Fields but
    //  the struct as a whole is seen as a multi byte field)
    if (FieldMeta->IsDynamic == false && FieldMeta->FieldOffset != 0) {
        uint8_t* field_address = (uint8_t*)packet + FieldMeta->FieldOffset;

        switch (FieldMeta->FieldSize) {
            case 2:
            {
                uint16_t* temp_2_bytes = (uint16_t*)field_address;
                temp_2_bytes[0] = bswap_16(*temp_2_bytes);
                break;
            }
            case 4:
            {
                uint32_t* temp_4_bytes = (uint32_t*)field_address;
                temp_4_bytes[0] = bswap_32(*temp_4_bytes);
                break;
            }
            case 8:
            {
                uint64_t* temp_8_bytes = (uint64_t*)field_address;
                temp_8_bytes[0] = bswap_64(*temp_8_bytes);
                break;
            }
            default: break;
        }
    }
}

void fix_packet_endianness(PACKET_METADATA* meta, void* _packet) {
    BASE_PACKET* packet = (BASE_PACKET*)_packet;
    if (packet->flags.endianness != GetEndianness()) {
        PacketFieldMetadata* FieldMeta;

        ITERATE_PRIMITIVE_DATA_TYPE(meta->Fields, pointer, FieldMeta) {
            fix_field_endianness(FieldMeta, packet);
        }
    }
}

BASE_PACKET* MultiByteDeserialize(PACKET_ID PacketId, OPAQUE_MEMORY mem) {
    BASE_PACKET* packet_res;
    PACKET_METADATA* PacketMeta = GetPacketMetadata(PacketId);
    packet_res = (BASE_PACKET*)malloc(PacketMeta->PacketSize);
    memcpy(packet_res, mem.Data, mem.Size);

    fix_packet_endianness(PacketMeta, packet_res);

    return packet_res;
}

void AdvancedDeallocate(BASE_PACKET* packet) {
    OPAQUE_MEMORY* mem;
    PACKET_METADATA* PacketMeta = GetPacketMetadata(packet->PacketId);

    PacketFieldMetadata* FieldMeta;
    ITERATE_PRIMITIVE_DATA_TYPE(PacketMeta->Fields, pointer, FieldMeta) {
        if (FieldMeta->IsDynamic) {
            mem = (OPAQUE_MEMORY*)((uint8_t*)packet + FieldMeta->FieldOffset);
            if (mem->Data != NULL) {
                Free(mem->Data);
            }
        }
    }
    Free(packet);
}

OPAQUE_MEMORY AdvancedSerialize(void* _packet) {
    BASE_PACKET* packet = (BASE_PACKET*)_packet;
    size_t TotalSize = 0;
    OPAQUE_MEMORY* Dynamicfield;
    OPAQUE_MEMORY MemResult;
    PACKET_METADATA* PacketMeta = GetPacketMetadata(packet->PacketId);
    void* field_address;
    uint8_t* MemoryIndex;

    // Calculate packet size
    PacketFieldMetadata* FieldMeta;
    ITERATE_PRIMITIVE_DATA_TYPE(PacketMeta->Fields, pointer, FieldMeta) {
        if (FieldMeta->IsDynamic) {
            field_address = (uint8_t*)packet + FieldMeta->FieldOffset;
            Dynamicfield = (OPAQUE_MEMORY*)field_address;
            TotalSize += sizeof(Dynamicfield->Size);
            TotalSize += Dynamicfield->Size;
        } else {
            TotalSize += FieldMeta->FieldSize;
        }
    }

    MemResult.Data = malloc(TotalSize);
    MemResult.Allocated = TRUE;
    MemResult.Size = TotalSize;

    MemoryIndex = (uint8_t*)MemResult.Data;

    // Fill in packet
    ITERATE_PRIMITIVE_DATA_TYPE(PacketMeta->Fields, pointer, FieldMeta) {
        field_address = (uint8_t*)packet + FieldMeta->FieldOffset;
        if (FieldMeta->IsDynamic) {
            Dynamicfield = (OPAQUE_MEMORY*)field_address;

            memcpy(MemoryIndex, &(Dynamicfield->Size), sizeof(Dynamicfield->Size));
            MemoryIndex += sizeof(Dynamicfield->Size);

            memcpy(MemoryIndex, Dynamicfield->Data, Dynamicfield->Size);
            MemoryIndex += Dynamicfield->Size;
        } else {
            memcpy(MemoryIndex, field_address, FieldMeta->FieldSize);
            MemoryIndex += FieldMeta->FieldSize;
        }
    }
    return MemResult;
}

BASE_PACKET* AdvancedDeserialize(PACKET_ID PacketId, OPAQUE_MEMORY mem) {
    BASE_PACKET* packet_res;
    void* field_address;
    OPAQUE_MEMORY* Dynamicfield;
    uint8_t* current_mem = (uint8_t*)mem.Data;

    PACKET_METADATA* PacketMeta = GetPacketMetadata(PacketId);
    packet_res = (BASE_PACKET*)malloc(PacketMeta->PacketSize);

    PacketFieldMetadata* FieldMeta;
    ITERATE_PRIMITIVE_DATA_TYPE(PacketMeta->Fields, pointer, FieldMeta) {
        field_address = (uint8_t*)packet_res + FieldMeta->FieldOffset;
        if (FieldMeta->IsDynamic) {
            Dynamicfield = (OPAQUE_MEMORY*)field_address;

            memcpy(&(Dynamicfield->Size), current_mem, sizeof(Dynamicfield->Size));
            current_mem += sizeof(Dynamicfield->Size);
            Dynamicfield->Data = malloc(Dynamicfield->Size);

            memcpy(Dynamicfield->Data, current_mem, Dynamicfield->Size);
            current_mem += Dynamicfield->Size;
        } else {
            memcpy(field_address, current_mem, FieldMeta->FieldSize);
            current_mem += FieldMeta->FieldSize;
        }
    }

    fix_packet_endianness(PacketMeta, packet_res);

    return packet_res;
}

BASE_PACKET* MultiByteAllocate(PACKET_ID PacketId) {
    BASE_PACKET* res;
    res = BasicAllocate(PacketId);
    res->flags.endianness = GetEndianness();
    return res;
}
