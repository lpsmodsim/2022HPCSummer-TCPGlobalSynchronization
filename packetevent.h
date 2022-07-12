#ifndef _packetEvent_H
#define _packetEvent_H

#include <sst/core/event.h>

enum PacketType {
    PACKET,
    LIMIT,
};

struct Packet {
    PacketType type; 
    int id;
    int node_id;
};

class PacketEvent : public SST::Event {

public:
    void serialize_order(SST::Core::Serialization::serializer &ser) override {
        Event::serialize_order(ser);
        ser & pack.type;
        ser & pack.id;
        ser & pack.node_id;
    }

    PacketEvent(Packet pack) :
        Event(),
        pack(pack)
    {}

    PacketEvent() {}

    Packet pack;

    ImplementSerializable(PacketEvent);
};



#endif