#ifndef _router_H
#define _router_H

#include <sst/core/component.h>
#include <sst/core/link.h>
#include "messageevent.h"
#include <queue>

class router : public SST::Component {

public:
    router( SST::ComponentId_t id, SST::Params& params );
    ~router();

    int verbose_level;
    bool tick ( SST::Cycle_t currentCycle ); 

    void setup();

    void commHandler(SST::Event *ev, int port);

    SST_ELI_REGISTER_COMPONENT(
        router,
        "tcpGlobalSync",
        "router",
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
        "description",
        COMPONENT_CATEGORY_UNCATEGORIZED
    )

    SST_ELI_DOCUMENT_PARAMS(
        {"tickFreq", "Descript", "1s"},
        {"numPorts", "Descript", "1"},   
        {"verbose_level", "Descript", "1"},
        {"queueSize", "Descript", "50"},
    )

    SST_ELI_DOCUMENT_PORTS(
        {"commPort%d", "A pointer to ports for connecting the router to flows.", {"StringEvent"}}
    )

private:
    SST::Output output;
    SST::Link **commPort; 
    int numPorts; 

    std::string clock;

    int queueSize;
    std::queue<Message> msgQueue;

    int limitEpsilon; // Integer duration before the router can send a new limit message. 
                       // Prevents router from constantly sending limit messages when queue becomes full
    int limitCount;

    float goodput;
    float throughput;
};

#endif