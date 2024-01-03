#include "DataPacketOperations.h"
#include "tests/MultiByte.h"

void multibyte_test(void) {
    PacketMultiByte*  mb_to_send;
    PacketMultiByte*  mb_received;
    PacketMultiByte*  mb_received2;

    OPAQUE_MEMORY       Memory1;
    OPAQUE_MEMORY       Memory2;

    PACKET_ID         mb_id;

    mb_id = GetPacketId("PacketMultiByte");

    // Test serialize-deserialize
    mb_to_send = (PacketMultiByte*)PacketAllocate(mb_id);
    mb_to_send->two_byte_field = 0x0123;
    mb_to_send->four_byte_field = 0x456789ab;
    mb_to_send->eight_byte_field = 0xcdef010123234545;

    Memory1 = PacketSerialize(mb_to_send);

    mb_received = (PacketMultiByte*)PacketDeserialize(Memory1);
    Free(Memory1.Data);

    assert(mb_to_send->two_byte_field == mb_received->two_byte_field);
    assert(mb_to_send->four_byte_field == mb_received->four_byte_field);
    assert(mb_to_send->eight_byte_field == mb_received->eight_byte_field);

    // Test auto endianness fix
    if (GetEndianness() == BigEndian) {
        mb_to_send->base.flags.endianness = LittleEndian;
    } else {
        mb_to_send->base.flags.endianness = BigEndian;
    }

    fix_packet_endianness(GetPacketMetadata(mb_id), (BASE_PACKET*)mb_to_send);

    assert(mb_to_send->two_byte_field == 0x2301);
    assert(mb_to_send->four_byte_field == 0xab896745);
    assert(mb_to_send->eight_byte_field == 0x454523230101efcd);

    Memory2 = PacketSerialize(mb_to_send);

    mb_received2 = (PacketMultiByte*)PacketDeserialize(Memory2);
    Free(Memory2.Data);

    assert(mb_received2->base.flags.endianness != mb_received->base.flags.endianness);
    assert(mb_received2->base.PacketId   == mb_received->base.PacketId);
    assert(mb_received2->two_byte_field == mb_received->two_byte_field);
    assert(mb_received2->four_byte_field == mb_received->four_byte_field);
    assert(mb_received2->eight_byte_field == mb_received->eight_byte_field);

    PacketDeallocate(mb_to_send);
    PacketDeallocate(mb_received);
    PacketDeallocate(mb_received2);
}

void MultiBytePacketSetup(void) {
    #define PACKET_DEF_FILE "PacketMultiByte.def"
    #include "PacketBody.c"
    #undef PACKET_DEF_FILE
}
