#ifndef _receivernode_H
#define _receivernode_H

#include <sst/core/component.h>
#include <sst/core/link.h>
#include <queue>
#include "packetevent.h"

class receivernode : public SST::Component {
public:
    receivernode( SST::ComponentId_t id, SST::Params& params );
    ~receivernode();

    bool tick ( SST::Cycle_t currentCycle );
    bool test ( SST::Cycle_t currentCycle );

    void eventHandler(SST::Event *ev);

    SST_ELI_REGISTER_COMPONENT(
        receivernode,
        "probGlobalSync",
        "receivernode",
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
        "Receives and processes packets from senders.",
        COMPONENT_CATEGORY_UNCATEGORIZED
    ) 

    SST_ELI_DOCUMENT_PARAMS(
        {"tickFreq", "Frequency object updates at", "1s"},
        {"process_rate", "Number of packets that are process per cycle", "10"},
        {"queue_size", "Size of queue", "50"},
        {"verbose_level", "Verbosity level for console output", "1"},
        {"num_nodes", "Number of nodes connected to receiver", "1"},
        {"window_size", "Amount of ticks in which the number of sender nodes that transmission rates are reset occur.", "1"}, 
    )

    SST_ELI_DOCUMENT_PORTS(
        {"port%d", "Port that packets are sent across.", {"PacketEvent"}}
    )

    SST_ELI_DOCUMENT_STATISTICS(
        {"queueSizeStat", "Size of Receiver's Queue", "int" , 1},
        // Throughput
        // Values entering queue
        // Queue oscillation
        // Average Queue Size
        // Link Utilization ?
    )

private:
    SST::Output output;
    SST::Link **port;

   

    std::string clock;
    std::queue<Packet> msgQueue;
    int queue_size;
    int process_rate;      
    int verbose_level;
    int num_nodes;
    int num_resets;

    float link_utilization;
    float packets_processed;
    int stat_window_size;
    int packet_loss;
    Statistic<int>* queueSizeStat;
};

#endif