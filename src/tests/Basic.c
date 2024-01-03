#include "tests/Basic.h"

void BasicTest(void) {
    PacketBasic*       BasicToSend;
    PacketBasic*       BasicReceived;

    OPAQUE_MEMORY       Memory;

    PACKET_ID         BasicId;

    BasicId = GetPacketId("PacketBasic");
    BasicToSend = (PacketBasic*)PacketAllocate(BasicId);

    BasicToSend->byte_1_field = 0x12;
    BasicToSend->byte_2_field = 0x34;
    BasicToSend->byte_3_field = 0x56;

    Memory = PacketSerialize(BasicToSend);

    BasicReceived = (PacketBasic*)PacketDeserialize(Memory);
    ClearOpaqueMemory(&Memory);

    assert(BasicToSend->byte_1_field == BasicReceived->byte_1_field);
    assert(BasicToSend->byte_2_field == BasicReceived->byte_2_field);
    assert(BasicToSend->byte_3_field == BasicReceived->byte_3_field);

    PacketDeallocate(BasicReceived);
    PacketDeallocate(BasicToSend);
}

void BasicPacketSetup(void) {
    #define PACKET_DEF_FILE "PacketBasic.def"
    #include "PacketBody.c"
    #undef PACKET_DEF_FILE
}