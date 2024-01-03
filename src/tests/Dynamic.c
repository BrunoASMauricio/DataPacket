#include "DataPacketOperations.h"
#include "tests/Dynamic.h"

void dynamic_test(void) {
    PacketDynamic*     dyn_to_send;
    PacketDynamic*     dyn_received;
    PacketDynamic*     dyn_received2;

    OPAQUE_MEMORY       Memory1;
    OPAQUE_MEMORY       Memory2;

    PACKET_ID         dyn_id;

    dyn_id = GetPacketId("PacketDynamic");

    // Test serialize-deserialize
    dyn_to_send = (PacketDynamic*)PacketAllocate(dyn_id);;
    dyn_to_send->two_byte_field = 0x0123;
    dyn_to_send->four_byte_field = 0x456789ab;
    const uint8_t random_data[10] = {0x01, 0x20, 0x03, 0x40, 0x05, 0x60, 0x07, 0x80, 0x09, 0xa0};

    uint8_t* target_data = (uint8_t*)malloc(ARRAY_SIZE(random_data));
    memcpy(target_data, random_data, ARRAY_SIZE(random_data));
    
    dyn_to_send->dyn_1 = CLOAK_MEMORY(ARRAY_SIZE(random_data), FALSE, (void*)target_data);
    dyn_to_send->two_byte_field_2 = 0xcdef;

    Memory1 = PacketSerialize(dyn_to_send);

    dyn_received = (PacketDynamic*)PacketDeserialize(Memory1);
    Free(Memory1.Data);

    assert(dyn_to_send->two_byte_field   == dyn_received->two_byte_field);
    assert(dyn_to_send->four_byte_field   == dyn_received->four_byte_field);
    assert(dyn_to_send->two_byte_field_2 == dyn_received->two_byte_field_2);
    assert(0 == memcmp(dyn_to_send->dyn_1.Data, random_data, ARRAY_SIZE(random_data)));

    // Test auto endianness fix
    if (GetEndianness() == BigEndian) {
        dyn_to_send->base.flags.endianness = LittleEndian;
    } else {
        dyn_to_send->base.flags.endianness = BigEndian;
    }

    fix_packet_endianness(GetPacketMetadata(dyn_id), (BASE_PACKET*)dyn_to_send);

    assert(dyn_to_send->two_byte_field == 0x2301);
    assert(dyn_to_send->four_byte_field == 0xab896745);
    assert(dyn_to_send->two_byte_field_2 == 0xefcd);

    Memory2 = PacketSerialize(dyn_to_send);

    dyn_received2 = (PacketDynamic*)PacketDeserialize(Memory2);
    Free(Memory2.Data);

    assert(dyn_received2->two_byte_field    == dyn_received->two_byte_field);
    assert(dyn_received2->four_byte_field    == dyn_received->four_byte_field);
    assert(dyn_received2->two_byte_field_2  == dyn_received->two_byte_field_2);
    assert(0 == memcmp(dyn_received->dyn_1.Data, dyn_received2->dyn_1.Data, dyn_received2->dyn_1.Size));

    PacketDeallocate(dyn_to_send);
    PacketDeallocate(dyn_received);
    PacketDeallocate(dyn_received2);

}

void DynamicPacketSetup(void) {
    #define PACKET_DEF_FILE "PacketDynamic.def"
    #include "PacketBody.c"
    #undef PACKET_DEF_FILE

}