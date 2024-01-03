#include "DataPacket.h"
#include "DataPacketOperations.h"
#include "BasicList.h"

// #include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

// static std::mutex PacketSetupLock;
pthread_mutex_t PacketSetupLock = PTHREAD_MUTEX_INITIALIZER;

LIST PacketDefinitionsList;
OPAQUE_MEMORY* PacketDefinitions;
LIST PacketOperationsList;
OPAQUE_MEMORY* PacketOperations;

// // A dynamic field (pointer type) doesnt' count as a multi byte field because
// //  the pointer itself is not part of the serialized packet
#define IS_MULTIBYTE(FieldMeta) (FieldMeta->FieldSize > 1 && \
                                 FieldMeta->IsDynamic == false)

endianness_t GetEndianness(void) {
    if (BYTE_ORDER == LITTLE_ENDIAN) {
        return LittleEndian;
    }
    return BigEndian;
}

inline PACKET_METADATA* GetPacketMetadata(PACKET_ID PacketId) {
    return ((PACKET_METADATA**)(PacketDefinitions->Data))[PacketId];
}

static inline PACKET_OPS GetPacketOps(PACKET_ID PacketId) {
    return ((PACKET_OPS*)(PacketOperations->Data))[PacketId];
}

PACKET_ID GetPacketId(const char* Name) {
    PACKET_METADATA* Obj;
    ITERATE_PRIMITIVE_DATA_TYPE(&PacketDefinitionsList, pointer, Obj) {
        if (0 == strcmp(Name, Obj->PacketName)) {
            return Obj->PacketId;
        }
    }
    assert("Unrecognized packet type" == 0);
    return 0;
}

void NewPacketFieldMetadata(LIST* FieldList, bool _IsDynamic,
                                const char* _FieldType, const char* _FieldName,
                                size_t _FieldOffset, size_t _FieldSize) {
    ALLOC_STRUCT(PacketFieldMetadata, NewFieldMetadata);
    NewFieldMetadata->IsDynamic = _IsDynamic;
    NewFieldMetadata->FieldType = _FieldType;
    NewFieldMetadata->FieldName = _FieldName;
    NewFieldMetadata->FieldOffset = _FieldOffset;
    NewFieldMetadata->FieldSize = _FieldSize;
    DataListInsert(FieldList, GENERIC_DATA(pointer, NewFieldMetadata));
}

PACKET_METADATA* NewPacketMetadata(const char* _PacketName,
                                    size_t _PacketSize) {
    PACKET_METADATA* NewMeta = (PACKET_METADATA*)calloc(sizeof(PACKET_METADATA), 1);

    NewMeta->PacketName = _PacketName;
    NewMeta->PacketSize = _PacketSize;
    NewMeta->Fields = AllocateList();
    return NewMeta;
}

BASE_PACKET* PacketAllocate(PACKET_ID PacketId) {
    return GetPacketOps(PacketId).allocate(PacketId);
}

void PacketDeallocate(void* _Packet) {
    BASE_PACKET* Packet = (BASE_PACKET*)_Packet;
    return GetPacketOps(Packet->PacketId).deallocate(Packet);
}

OPAQUE_MEMORY PacketSerialize(void* _Packet) {
    BASE_PACKET* Packet = (BASE_PACKET*)_Packet;
    return GetPacketOps(Packet->PacketId).serialize(Packet);
}

BASE_PACKET* PacketDeserialize(OPAQUE_MEMORY Mem) {
    BASE_PACKET* Packet = (BASE_PACKET*)Mem.Data;
    return GetPacketOps(Packet->PacketId).deserialize(Packet->PacketId, Mem);
}

void FinalizePacket(PACKET_METADATA* Meta){
    pthread_mutex_lock(&PacketSetupLock);

    Meta->PacketFlags = 0x0;
    PacketFieldMetadata* FieldMeta;
    ITERATE_PRIMITIVE_DATA_TYPE(Meta->Fields, pointer, FieldMeta) {
        if(FieldMeta->IsDynamic == true) {
            SET_BIT_AT(Meta->PacketFlags, DYNAMIC_FIELDS);
        }

        if (IS_MULTIBYTE(FieldMeta) &&
            strcmp(FieldMeta->FieldType, "BASE_PACKET") != 0) {
            SET_BIT_AT(Meta->PacketFlags, MUlTI_BYTE_FIELDS);
        }
    }

    ALLOC_STRUCT(PACKET_OPS, PacketOps);

    PacketOps->allocate       = BasicAllocate;
    PacketOps->deallocate     = BasicDeallocate;
    PacketOps->serialize      = BasicSerialize;
    if (Meta->PacketFlags == 0x0) {
        PacketOps->deserialize    = BasicDeserialize;
    } else {
        if (CHECK_BIT_AT(Meta->PacketFlags, MUlTI_BYTE_FIELDS)) {
            PacketOps->allocate      = MultiByteAllocate;
            PacketOps->deserialize   = MultiByteDeserialize;
        }

        if (CHECK_BIT_AT(Meta->PacketFlags, DYNAMIC_FIELDS)) {
            PacketOps->serialize      = AdvancedSerialize;
            // includes multi_byte endianness fix
            PacketOps->deserialize    = AdvancedDeserialize;
            PacketOps->deallocate     = AdvancedDeallocate;
        }
    }

    Meta->PacketId = PacketOperationsList.Length;
    // Pass PacketOps memory responsibility into list
    MemoryListInsert(&PacketOperationsList, \
                     CLOAK_MEMORY(sizeof(PACKET_OPS), TRUE, PacketOps));

    assert(PacketDefinitionsList.Length == PacketOperationsList.Length);

    pthread_mutex_unlock(&PacketSetupLock);
}

void FinalizePackets(void) {
    PacketDefinitions = SerializeDataList(&PacketDefinitionsList);
    // we can now clear the lists
    PacketOperations = SerializeMemoryListElements(&PacketOperationsList);
}

void ReleasePackets(void) {
    // Release packet metadata
    PACKET_METADATA* PacketMeta = NULL;
    ITERATE_PRIMITIVE_DATA_TYPE(&PacketDefinitionsList, pointer, PacketMeta) {
        PacketFieldMetadata* FieldMeta;
        ITERATE_PRIMITIVE_DATA_TYPE(PacketMeta->Fields, pointer, FieldMeta) {
            Free(FieldMeta);
        }
        FreeDataList(PacketMeta->Fields);
        Free(PacketMeta);
    }
    ClearDataList(&PacketDefinitionsList);

    FreeOpaqueMemory(PacketDefinitions);

    // Release packet operations
    ClearMemoryList(&PacketOperationsList);

    FreeOpaqueMemory(PacketOperations);
}

STRING* PacketToString(PACKET_METADATA* PacketMeta) {
    STRING* Str = FormatString("packet: %s (%d) id: %d of types: ",
                               PacketMeta->PacketName, PacketMeta->PacketSize,
                               PacketMeta->PacketId);

    if (PacketMeta->PacketFlags == 0x0) {
        StringConcatCharArr(Str, "nothing");
    } else {
        if (CHECK_BIT_AT(PacketMeta->PacketFlags, DYNAMIC_FIELDS)) {
            StringConcatCharArr(Str, "dynamic ");
        }

        if (CHECK_BIT_AT(PacketMeta->PacketFlags, MUlTI_BYTE_FIELDS)) {
            StringConcatCharArr(Str, "multi-byte ");
        }
    }

    StringConcatCharArr(Str, "\n");

    PacketFieldMetadata* FieldMeta;
    ITERATE_PRIMITIVE_DATA_TYPE(PacketMeta->Fields, pointer, FieldMeta) {
        StringFormatAppend(Str, "\t%s %s" "\n\t\tdynamic: %s" "\n\t\tsize: %ld"\
                           "\n\t\toffset: %ld\n",
                           FieldMeta->FieldType, FieldMeta->FieldName,
                           FieldMeta->IsDynamic ? "true" : "false",
                           FieldMeta->FieldSize, FieldMeta->FieldOffset);
    }

    StringConcatCharArr(Str, "\n");

    return Str;
}

// string packet_to_string(void* _packet) {
//     BASE_PACKET* packet = (BASE_PACKET*)_packet;
//     PACKET_METADATA* PacketMeta = GetPacketMetadata(packet->PacketId);
//     string str = "packet: " + string(PacketMeta->PacketName);
//     str += " (" + to_string(PacketMeta->PacketSize) + ") id: ";
//     str += to_string(PacketMeta->PacketId) + "\n";

//     for(PacketFieldMetadata FieldMeta: *PacketMeta->Fields) {
//         void* field_address = (uint8_t*)packet + FieldMeta.FieldOffset;
//         if (FieldMeta.IsDynamic) {
//             OPAQUE_MEMORY* mem = (OPAQUE_MEMORY*)field_address;
//             str += "\tdynamic field " + string(FieldMeta.FieldType);
//             str += " " + string(FieldMeta.FieldName);
//             str += " [" + to_string(mem->size) + "] {";
//             for (size_t i = 0; i < mem->size; i++) {
//                 char buf[3];
//                 snprintf(buf, sizeof(buf), "%02x", ((uint8_t*)(mem->data))[i]);
//                 str += "0x" + string(buf) + " ";
//             }
//             str += "}\n";
//         } else {
//             char buf[17];
//             str += "\t" + string(FieldMeta.FieldType);
//             str += " " + string(FieldMeta.FieldName) + " = 0x";
//             switch (FieldMeta.FieldSize) {
//                 case 1:
//                 {
//                     uint8_t* temp_1_byte = (uint8_t*)field_address;
//                     snprintf(buf, sizeof(buf), "%02x", *temp_1_byte);
//                     str += string(buf);
//                     break;
//                 }
//                 case 2:
//                 {
//                     uint16_t* temp_2_bytes = (uint16_t*)field_address;
//                     snprintf(buf, sizeof(buf), "%04x", *temp_2_bytes);
//                     str += string(buf);
//                     break;
//                 }
//                 case 4:
//                 {
//                     uint32_t* temp_4_bytes = (uint32_t*)field_address;
//                     snprintf(buf, sizeof(buf), "%08x", *temp_4_bytes);
//                     str += string(buf);
//                     break;
//                 }
//                 case 8:
//                 {
//                     uint64_t* temp_8_bytes = (uint64_t*)field_address;
//                     snprintf(buf, sizeof(buf), "%016lx", *temp_8_bytes);
//                     str += string(buf);
//                     break;
//                 }
//             }
//             str += "\n";

//         }
//     }
//     str += "\n";
//     return str;
// }

STRING* PacketsToString(void){
    STRING* Str = GenEmptyString();
    PACKET_METADATA* PacketMeta = NULL;
    ITERATE_PRIMITIVE_DATA_TYPE(&PacketDefinitionsList, pointer, PacketMeta) {
        STRING* PacketString = PacketToString(PacketMeta);
        StringConcatString(Str, PacketString);
        FreeString(PacketString);
    }
    return Str;
}






