#ifndef _client_H
#define _client_H

#include <sst/core/component.h>
#include <sst/core/link.h>
#include "messageevent.h"

#define IDLE 0      // Not sending a packet
#define WAITING 1   // Waiting for an acknowledgement.
#define SENDING 2   // Sending a frame.

#define SEND_NEW 0  // Sending a new frame.
#define SEND_DUP 1  // Sending a duplicate frame.

class client : public SST::Component {

public:
    client ( SST::ComponentId_t id, SST::Params& params );
    ~client();

    bool tick ( SST::Cycle_t currentCycle );

    void commHandler(SST::Event *ev);
    int verbose_level;

    SST_ELI_REGISTER_COMPONENT(
        client,
        "tcpGlobalSync",
        "client",
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
        "Client component which sends frames over to a router.",
        COMPONENT_CATEGORY_UNCATEGORIZED
    )

    SST_ELI_DOCUMENT_PARAMS(
        {"tickFreq", "The frequency that the client updates and sends a new frame.", "1s"},
        {"timeout", "The amount of cycles before a client will resend any unacknowledge frames.", "100"}, 
        {"verbose_level", "Level of verbosity for console output.", "1"},
        {"node_id", "ID of the client/node.", "1"},
    )

    SST_ELI_DOCUMENT_PORTS(
        {"commPort", "Port that frames and acknowledgements are sent across.", {"MessageEvent"}}
    )

private:
    SST::Output output; // SST Object for console output.
    SST::Link *commPort;    // Pointer to a component port.

    std::string clock;  // Node's clock which accepts unit time.
    int timeout; // The amount of cycles before the node sends retransmissions.

    void sendMessage(MessageType type, StatusType status, int node, int frame);
    void checkForSend(SST::Cycle_t currentCycle);

    int frames_to_send; // Total frames that the node will send for its packet.
    int current_frame;  // The current frame in the packet that the node is at.
    int acks_received;  // The amount of acknowledgements that the node has for the packet it is sending.
    int start_send_cycle;   // The cycle in which the node begins to send a packet of frames.
    int window_size;    // The window size the node sends at.
    int timer_start;    // The cycle that the time out timer starts at. 

    int send_state;     // Is the node sending new packets or retransmissions.
    int client_state;   // Is the node IDLE, WAITING, or SENDING.
    int node_id;        // ID of node/client.
     
    int test;
};

#endif