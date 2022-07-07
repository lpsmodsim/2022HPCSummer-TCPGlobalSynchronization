#include <sst/core/sst_config.h>
#include "router.h"

router::router( SST::ComponentId_t id, SST::Params& params ) : SST::Component(id) {
    // Initialize Parameters
    clock = params.find<std::string>("tickFreq", "1s");
    verbose_level = params.find<int64_t>("verbose_level", 1);
    numPorts = params.find<int64_t>("numPorts", 1); 
    queueSize = params.find<int64_t>("queueSize", 50);

    output.init(getName() + "->", verbose_level, 0, SST::Output::STDOUT); 

    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

    registerClock(clock, new SST::Clock::Handler<router>(this, &router::tick));
 
    // Allocate space for number of ports.
    commPort = new SST::Link*[numPorts];

    // Configure all ports to have the same event handler.
    for (int i = 0; i < numPorts; ++i) {
        std::string port = "commPort" + std::to_string(i);
        commPort[i] = configureLink(port, new SST::Event::Handler<router, int>(this, &router::commHandler, i));

        if ( !commPort[i] ) {
            output.fatal(CALL_INFO, -1, "Commport%d failed.\n", i);
        }
    }  
}

router::~router() {

}

void router::setup() {
    goodput = 0;
    throughput = 0;
}

bool router::tick( SST::Cycle_t currentCycle ) {
    output.verbose(CALL_INFO, 2, 0, "Goodput: %f\nThroughput: %f\n", goodput, throughput);

    // Potential statistics
    output.verbose(CALL_INFO, 2, 0, "Queue Size: %ld\n", msgQueue.size()); 
    std::cout << goodput / throughput << std::endl; 

    // Hardstop for simulation
    if (currentCycle == 100) {
        primaryComponentOKToEndSim();
        return(true);
    } 

    // Check if queue is empty.
    if (!msgQueue.empty()) {
        Message msg = msgQueue.front(); // Grab message at front

        msg.type = ACK; // Change msg to ack
        commPort[msg.node]->send(new MessageEvent(msg)); // Send ack back to node
        output.verbose(CALL_INFO, 3, 0, "Sent ACK for Frame %d for Node %d\nWhich is %d (NEW 1/DUP 0)\n", msg.id, msg.node, msg.status);

        // Update statistics.
        if (msg.status == NEW) {
            goodput--;
            throughput--;
        } else {
            throughput--;
        }

        msgQueue.pop(); // Remove message from queue.
    }

    return(false);
}

void router::commHandler(SST::Event *ev, int port) {
    MessageEvent *me = dynamic_cast<MessageEvent*>(ev);
    if (me != NULL) {
        switch (me->msg.type) {
            case FRAME:
                output.verbose(CALL_INFO, 3, 0, "Received frame %d from node %d\n", me->msg.id, me->msg.node);

                if (msgQueue.size() + 1 > queueSize) {
                    output.verbose(CALL_INFO, 3, 0, "Dropped frame %d from node %d\n", me->msg.id, me->msg.node);
                } else {             
                    // Update statistics
                    if (me->msg.status == NEW) {
                        goodput++;
                        throughput++;
                    } else {
                        throughput++;
                    }
                   msgQueue.push(me->msg); // Push message onto queue.
                }
                break;

            case ACK:
                output.fatal(CALL_INFO, -1, "Router should not be receiving acks. Error!");
                break;
        }
    } 
    delete ev;
}