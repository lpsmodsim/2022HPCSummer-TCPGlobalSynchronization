#ifndef _messsageEvent_H
#define _messsageEvent_H

#include <sst/core/event.h>

enum MessageType {
    FRAME,
    LIMIT,
    ACK,
};

enum StatusType {
    DUP,
    NEW,
};

struct Message {
    MessageType type;
    StatusType status;
    int node;
    int id; 
};

class MessageEvent : public SST::Event {

public:
    void serialize_order(SST::Core::Serialization::serializer &ser) override {
        Event::serialize_order(ser);
        ser & msg.type;
        ser & msg.status;
        ser & msg.node;
        ser & msg.id; 
    }

    MessageEvent(Message msg) :
        Event(),
        msg(msg)
    {}

    MessageEvent() {}

    Message msg;

    ImplementSerializable(MessageEvent);
};



#endif