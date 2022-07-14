#include <sst/core/sst_config.h>
#include <sst/core/simulation.h>
#include "receiver.h"

receiver::receiver( SST::ComponentId_t id, SST::Params& params ) : SST::Component(id) {

    // Parameters
    clock = params.find<std::string>("tickFreq", "1s");
    process_rate = params.find<int64_t>("process_rate", 10);
    queue_size = params.find<int64_t>("queue_size", 50);
    verbose_level = params.find<int64_t>("verbose_level", 1);
    num_nodes = params.find<int64_t>("num_nodes", 1);
    window_size = params.find<int64_t>("window_size", 100);

    // Enabling SST Output
    output.init(getName() + "->", verbose_level, 0, SST::Output::STDOUT);

    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

    // Variables
    sampling_start_time = 0;
    sampling_status = false;
    already_sampled = false;
    nodes_limited = 0;
    globsync_detect = 0;
    
    // Stats 
    packet_loss = 0;
    packets_processed = 0;
    link_utilization = 0;

    // Ports
    port = new SST::Link*[num_nodes];

    for (int i = 0; i < num_nodes; ++i) {
        std::string strport = "port" + std::to_string(i);
        port[i] = configureLink(strport, new SST::Event::Handler<receiver>(this, &receiver::eventHandler));
        if (!port) {
            output.fatal(CALL_INFO, -1, "Failed to configure port 'port'\n");
        } 
    } 

    // Clock
    registerClock(clock, new SST::Clock::Handler<receiver>(this, &receiver::tick));   
}

receiver::~receiver() {

}

void receiver::setup() {
    tracked_nodes[0] = 0;
    tracked_nodes[1] = 0;
    tracked_nodes[2] = 0;
}

bool receiver::tick( SST::Cycle_t currentCycle ) {
    output.verbose(CALL_INFO, 2, 0, "SimTime: %ld\n", getCurrentSimTime());
    output.verbose(CALL_INFO, 2, 0, "Queue Size: %ld\n", msgQueue.size());
    output.verbose(CALL_INFO, 2, 0, "Global Synchronization Percentage: %f\n\n", globsync_detect);

    // Statistic Info 
    //^^^std::cout << msgQueue.size() << std::endl;
    //^^^std::cout << packet_loss << std::endl;
    //^^^std::cout << link_utilization * 100 << std::endl; 
    //^^^std::cout << globsync_detect << std::endl;

    if (sampling_status == true && (getCurrentSimTimeMilli() >= sampling_start_time + window_size)) {
        output.verbose(CALL_INFO, 2, 0, "Ending Sampling^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        if (already_sampled == false) {
            if (globsync_detect != 0) {
                globsync_detect -= 0.25;
            } 
        } 
        already_sampled = false;
        sampling_status = false; 
        sampling_start_time = 0;
        nodes_limited = 0;
        for (int i = 0; i < num_nodes; i++) {
            tracked_nodes[i] = 0;
        }
    }

    if (currentCycle == 600) {
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

void receiver::eventHandler(SST::Event *ev) {
    PacketEvent *pe = dynamic_cast<PacketEvent*>(ev);
    if (pe != NULL) {
        switch (pe->pack.type) {
            case PACKET: 
            
                // Check if queue is full
                if (msgQueue.size() + 1 > queue_size) {
                    output.verbose(CALL_INFO, 3, 0, "Packet Loss from %d\n", pe->pack.node_id);
                    // Send limit message alerting sender that packet was dropped.
                    Packet limitMsg = { LIMIT, pe->pack.id, pe->pack.node_id };
                    port[limitMsg.node_id]->send(new PacketEvent(limitMsg));
                    packet_loss++;

                    // Packet loss occurs
                
                     
                    // If dropped packet from previous tracked node_id, ignore

                    // If dropped packet from a new tracked node_id, increase chance of global sync occuring
                    // Store chance and wait for next sample for window.
                    // Drop packet occurs again and begin sampling.
                } else {
                    // Add message to queue
                    msgQueue.push(pe->pack);
                }
                break;
            case LIMIT:
                // Receives message that rates are limited, 

                if (sampling_status == false && already_sampled == false) {
                    output.verbose(CALL_INFO, 2, 0, "Started Sampling----------------------------------------------------------------------------\n");
                    sampling_start_time = getCurrentSimTimeMilli(); // Begin Window of Time
                    sampling_status = true; // Start sampling.
                    tracked_nodes[pe->pack.node_id] = 1;
                    nodes_limited++;
                } 

                if (sampling_status == true && tracked_nodes[pe->pack.node_id] == 0 && already_sampled == false) {
                    output.verbose(CALL_INFO, 2, 0, "Sampling: Packet Loss from %d\n", pe->pack.node_id);
                    tracked_nodes[pe->pack.node_id] = 1;
                    nodes_limited++; 
                    if (nodes_limited == num_nodes) {
                        output.verbose(CALL_INFO, 2, 0, "Ending Sampling-----------------------------------------\n");
                        if (globsync_detect < 1) {
                            globsync_detect += 0.25; 
                        } 
                        already_sampled = true; 
                        nodes_limited = 0;
                        for (int i = 0; i < num_nodes; i++) {
                            tracked_nodes[i] = 0;
                        }
                    }
                }
                //output.fatal(CALL_INFO, -1, "Receiver should not be receiving limit messages. Error!\n");
                break;
        }
    }
    delete ev;
}