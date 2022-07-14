#ifndef _sender_H
#define _sender_H

#include <sst/core/component.h>
#include <sst/core/link.h>
#include "packetevent.h"

class sender : public SST::Component {
public:
    sender( SST::ComponentId_t id, SST::Params& params );
    ~sender();

    bool tick ( SST::Cycle_t currentCycle );

    void eventHandler(SST::Event *ev);

    SST_ELI_REGISTER_COMPONENT(
        sender,         // Component Name
        "tcpGlobSync",   // Element Name
        "sender",       // Component Name
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),     // Element Version
        "Sends packets via a link to a receiver with a limited queue.",     //Component Description
        COMPONENT_CATEGORY_UNCATEGORIZED    // Component Category
    ) 

    // Parameter Name, Description, Default Value
    SST_ELI_DOCUMENT_PARAMS(
        {"tickFreq", "Frequency object updates at", "1s"},
        {"low_send_rate", "Lower limit of transmission rate", "10"},
        {"max_send_rate", "Max limit of transmission rate", "100"},
        {"verbose_level", "Verbosity level of console output", "1"},
        {"node_id", "ID of sender node connected to receiving node", "1"},
        {"starting_cycle", "Cycle when sender will begin transmitting packets", "1"}
    )

    // Port Name, Description, Event Type
    SST_ELI_DOCUMENT_PORTS(
        {"port", "Port that packets are sent across.", {"PacketEvent"}}
    )

private:
    SST::Output output;     // SST Object for console output.
    SST::Link *port;        // Pointer to a port.
    SST::TimeConverter *tc; // Required for converted sim time and component time.

    void finish(); 
    inline void sendPacket(int id, int delay);  // Constructs and sends a packet via the port.

    int send_rate_data[600];
    int counter = 0;
    int drop_counter = 0;
    int rate_drop[1000];

    std::string clock;      // Frequency component will tick at. Takes in UnitAlgebra.
    int low_send_rate;      // Starting rate and what node will lower transmission rate to during packet loss.
    int max_send_rate;      // Maximum transmission rate of a node.
    int curr_send_rate;     // # of packets that are being sent per tick.
    int packets_to_send;    // # of packets that are sent per tick.
    int verbose_level;      // Console output verbosity parameter.
    int node_id;            // Component ID in simulation composition.
    int starting_cycle;     // Cycle that node begins transmitting data.

    int stat_window_size;   // Window Size
    int send_delay;         // Packet sending delay. 
};


#endif