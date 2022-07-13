#include <sst/core/sst_config.h>
#include <sst/core/simulation.h>
#include "sender.h"


sender::sender( SST::ComponentId_t id, SST::Params& params ) : SST::Component(id) {

    // Parameters
    clock = params.find<std::string>("tickFreq", "1s");
    low_send_rate = params.find<int64_t>("low_send_rate", 10);
    max_send_rate = params.find<int64_t>("max_send_rate", 100);
    verbose_level = params.find<int64_t>("verbose_level", 1);
    node_id = params.find<int64_t>("node_id", 0);
    starting_cycle = params.find<int64_t>("starting_cycle", 1); 

    // Variables
    curr_send_rate = low_send_rate - 1;
    counter = 0;
    drop_counter = 0;
    send_delay = 0;

    output.init(getName() + "->", verbose_level, 0, SST::Output::STDOUT);
 
    registerClock(clock, new SST::Clock::Handler<sender>(this, &sender::tick));
    port = configureLink("port", new SST::Event::Handler<sender>(this, &sender::eventHandler));
    if (!port) {
        output.fatal(CALL_INFO, -1, "Failed to configure port 'port'\n");
    } 
}

sender::~sender() {

}

void sender::finish() {
    /**for (int i = 0; i < 300; i++) {
        std::cout << send_rate_data[i] << std::endl;
    }*/
    /**for (int i = 0; i < drop_counter; i++) {
        std::cout << rate_drop[i] << std::endl;
    }*/
    std::cout << "End Data For " << node_id << std::endl;
    
}

bool sender::tick( SST::Cycle_t currentCycle ) { 
    if (currentCycle >= starting_cycle) {
        if (curr_send_rate < max_send_rate) {
            curr_send_rate = curr_send_rate + 1; // Increase transmission rate per tick.
        }
        
        output.verbose(CALL_INFO, 2, 0, "Sending %d\n", curr_send_rate);

        // Statistic Info
        //^^^std::cout << node_id << ":" << curr_send_rate << std::endl; 
        send_rate_data[counter] = curr_send_rate;
        counter++;

        packets_to_send = curr_send_rate;

        for (int i = 0; i < packets_to_send; i++) {
            output.verbose(CALL_INFO, 3, 0, "Sending Packet #%d\n", i);
            sendPacket(i, send_delay); 
            send_delay += i;
        }
    } else {
        // output 0 as curr send rate for statistics
        //^^^std::cout << node_id << ":" << 0 << std::endl;
        send_rate_data[counter] = 0;
        counter++;
    }

    send_delay = 0;
    return(false);
}

void sender::eventHandler(SST::Event *ev) {
    PacketEvent *pe = dynamic_cast<PacketEvent*>(ev);
    if ( pe != NULL ) {
        switch (pe->pack.type) {
            case PACKET:
                output.fatal(CALL_INFO, -1, "Sender should not receive packets. Error!\n");
                break;
            case LIMIT:
                output.verbose(CALL_INFO, 2, 0, "Packets were dropped. Limiting transmission rate\n");

                curr_send_rate = low_send_rate; // Limit transmission rate
                
                port->send(new PacketEvent(pe->pack));
                // Start delay?

                // Statistic when node dropped a packet.
                //^^^std::cout << node_id << ":" << getCurrentSimTimeMilli() << ":" << 1 << std::endl;
                //^^^std::cout << getCurrentSimTimeMilli() << std::endl;
                //rate_drop[drop_counter] = getCurrentSimTime();
                //drop_counter++;
        }
    }
    delete ev;
}

void sender::sendPacket(int id, int delay) {
    PacketType type = PACKET;
    Packet packet = { type, id, node_id};

    port->send(delay, tc, new PacketEvent(packet));
}