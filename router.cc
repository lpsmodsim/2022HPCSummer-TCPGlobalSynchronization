#include <sst/core/sst_config.h>
#include "router.h"

router::router( SST::ComponentId_t id, SST::Params& params ) : SST::Component(id) {
    // Initialize Parameters
    clock = params.find<std::string>("tickFreq", "1s");
    verbose_level = params.find<int64_t>("verbose_level", 1);
    numPorts = params.find<int64_t>("numPorts", 1); 

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
    limitCount = 0;
    queueSize = 50;
    limitEpsilon = 25;
}

bool router::tick( SST::Cycle_t currentCycle ) {
    output.verbose(CALL_INFO, 2, 0, "Goodput: %f\nThroughput: %f\n", goodput, throughput);
    limitCount++;
    // Potential statistics
    //std::cout << msgQueue.size() << std::endl;
    //std::cout << goodput / throughput << std::endl;

    if (currentCycle == 100) {
        primaryComponentOKToEndSim();
        return(true);
    }
    //output.verbose(CALL_INFO, 1, 0, "Testing...\n"); 

    if (!msgQueue.empty()) {
        Message msg = msgQueue.front(); // Grab message at front

        msg.type = ACK; // Change msg to ack
        commPort[msg.node]->send(new MessageEvent(msg)); // Send ack back to node
        output.verbose(CALL_INFO, 3, 0, "Sent ACK for Frame %d for Node %d\nWhich is %d (NEW 1/DUP 0)\n", msg.id, msg.node, msg.status);

        if (msg.status == NEW) {
            goodput--;
            throughput--;
        } else {
            throughput--;
        }

        msgQueue.pop();
    }

    return(false);
}

void router::commHandler(SST::Event *ev, int port) {
    MessageEvent *me = dynamic_cast<MessageEvent*>(ev);
    if (me != NULL) {
        switch (me->msg.type) {
            case FRAME:
                output.verbose(CALL_INFO, 3, 0, "Received frame %d from node %d\n", me->msg.id, me->msg.node);

                if (me->msg.status == NEW) {
                    goodput++;
                    throughput++;
                } else {
                    throughput++;
                }


                if (msgQueue.size() + 1 > queueSize && limitEpsilon < limitCount) {
                    // Send out msgs to all clients to slow down window size
                    Message limitMsg = { LIMIT, NEW, 0, 0 };
                    output.verbose(CALL_INFO, 3, 0, "Sending Limit Messages Out\n");
                    for (int i = 0; i < numPorts; ++i) {
                        commPort[i]->send(new MessageEvent(limitMsg));
                    }
                    // drop message.
                } else {
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