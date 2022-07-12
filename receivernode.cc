#include <sst/core/sst_config.h>
#include <sst/core/simulation.h>
#include "receivernode.h"

receivernode::receivernode( SST::ComponentId_t id, SST::Params& params ) : SST::Component(id) {

    // Parameters
    clock = params.find<std::string>("tickFreq", "1s");
    process_rate = params.find<int64_t>("process_rate", 10);
    queue_size = params.find<int64_t>("queue_size", 50);
    verbose_level = params.find<int64_t>("verbose_level", 1);
    num_nodes = params.find<int64_t>("num_nodes", 1);

    // Enabling SST Output
    output.init(getName() + "->", verbose_level, 0, SST::Output::STDOUT);

    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

    // Stats
    queueSizeStat = registerStatistic<int>("queueSizeStat");
    packet_loss = 0;
    packets_processed = 0;
    link_utilization = 0;

    // Ports
    port = new SST::Link*[num_nodes];

    for (int i = 0; i < num_nodes; ++i) {
        std::string strport = "port" + std::to_string(i);
        port[i] = configureLink(strport, new SST::Event::Handler<receivernode>(this, &receivernode::eventHandler));
        if (!port) {
            output.fatal(CALL_INFO, -1, "Failed to configure port 'port'\n");
        } 
    }

    std::string secondClock = "1ms";

    // Clock
    registerClock(clock, new SST::Clock::Handler<receivernode>(this, &receivernode::tick));  
    registerClock(secondClock, new SST::Clock::Handler<receivernode>(this, &receivernode::test)); 
}

receivernode::~receivernode() {

}

bool receivernode::tick( SST::Cycle_t currentCycle ) {
    output.verbose(CALL_INFO, 2, 0, "SimTime: %ld\n", getCurrentSimTime());
    output.verbose(CALL_INFO, 2, 0, "Queue Size: %ld\n", msgQueue.size());

    // Statistic Info
    //^^^queueSizeStat->addData(msgQueue.size());
    std::cout << msgQueue.size() << std::endl;
    //^^^std::cout << packet_loss << std::endl;
    //^^^std::cout << link_utilization * 100 << std::endl;
    performStatisticOutput(queueSizeStat);

    if (currentCycle == 800) {
        primaryComponentOKToEndSim();
        return(true);
    }

    // Check if message queue is empty
    if (!msgQueue.empty()) {
        // Process messages in queue.
        for (int i = 0; i < process_rate; i++) {
            if (msgQueue.empty()) {
                break; // If the queue becomes empty during processing, break out.
            }
        packets_processed++;
        
        msgQueue.pop(); // "Process" the packet and remove it from queue.
        }
    }

    link_utilization = packets_processed / (float) process_rate;
    packets_processed = 0;

    return(false);
}

void receivernode::eventHandler(SST::Event *ev) {
    PacketEvent *pe = dynamic_cast<PacketEvent*>(ev);
    if (pe != NULL) {
        switch (pe->pack.type) {
            case PACKET: 
            
                // Check if queue is full
                if (msgQueue.size() + 1 > queue_size) {
                    // Send limit message alerting sender that packet was dropped.
                    Packet limitMsg = { LIMIT, pe->pack.id, pe->pack.node_id };
                    port[limitMsg.node_id]->send(new PacketEvent(limitMsg));
                    packet_loss++;

                } else {
                    // Add message to queue
                    msgQueue.push(pe->pack);
                }
                break;
            case LIMIT:
                output.fatal(CALL_INFO, -1, "Receiver should not be receiving limit messages. Error!\n");
                break;
        }
    }
    delete ev;
}

bool receivernode::test(SST::Cycle_t currentCycle) { 
    return(false);
}