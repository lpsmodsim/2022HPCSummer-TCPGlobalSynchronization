#ifndef _receiver_H
#define _receiver_H

#include <sst/core/component.h>
#include <sst/core/link.h>
#include <queue>
#include "packetevent.h"

#define NUM_NODES 3

class receiver : public SST::Component {
public:
    receiver( SST::ComponentId_t id, SST::Params& params );
    ~receiver();

    bool tick ( SST::Cycle_t currentCycle ); 

    void eventHandler(SST::Event *ev);

    void setup();

    SST_ELI_REGISTER_COMPONENT(
        receiver,       // Component Name
        "tcpGlobSync",   // Element Name
        "receiver",     // Component Name
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),     // Element Version
        "Receives and processes packets from senders.",     // Component Description
        COMPONENT_CATEGORY_UNCATEGORIZED    // Component Category
    ) 

    // Parameter Name, Description, Default Value
    SST_ELI_DOCUMENT_PARAMS(
        {"tickFreq", "Frequency object updates at", "1s"},
        {"process_rate", "Number of packets that are process per cycle", "10"},
        {"queue_size", "Size of queue", "50"},
        {"verbose_level", "Verbosity level for console output", "1"},
        {"num_nodes", "Number of nodes connected to receiver", "1"},
        {"window_size", "Amount of ticks in which the number of sender nodes that transmission rates are reset occur.", "1"}, 
    )

    // Port Name, Description, Event Type
    SST_ELI_DOCUMENT_PORTS(
        {"port%d", "Port that packets are sent across.", {"PacketEvent"}}
    ) 

private:
    SST::Output output; // SST Object for console output.
    SST::Link **port;   // Pointer to multiple ports.

    std::string clock;  // Frequency component will tick at. Takes in UnitAlgebra.
    std::queue<Packet> msgQueue;    // Queue for packets.
    int queue_size;     // Size of queue.
    int process_rate;   // Amount of packets that can processed per tick.
    int verbose_level;  // Verbosity level of console output.
    int num_nodes;      // Number of connected senders.
    int num_resets;     //

    float link_utilization;     // Aggregate link utilization of the receiver.
    float packets_processed;    // Packets processed per tick.
    int packet_loss;    // Number of packets loss. 

    int window_size;    // Window size for synchronization detection. 
    int sampling_start_time; // Simulator Time in which sampling occurs. 
    bool sampling_status;
    bool already_sampled;
    int tracked_nodes[NUM_NODES];
    int nodes_limited;
    float globsync_detect;
};

#endif