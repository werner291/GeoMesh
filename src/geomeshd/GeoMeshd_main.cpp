//
// Created by Werner Kroneman on 28-01-16.
//

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>
#include <unistd.h>

using namespace std;

#ifdef __linux__
#include "TunnelDeliveryInterface_Linux.h"
#endif
#ifdef __APPLE__
#include "TunnelDeliveryInterface_Apple.h"
#endif

#include "../UniqueAddress.h"

#include "../Logger.h"

#include "UDPManager.h"

#include "../Router.h"


int main(int argc, char **argv) {

    char* givenAddress = NULL;

    std::vector< std::string > peers;

    int udpPort = 10976;

    for (int i=0; i < argc; i++) {
        if (strcmp(argv[i], "-ip6") == 0) {
            givenAddress = argv[++i];
        }

        if (strcmp(argv[i], "-peer") == 0) {
            peers.emplace_back(std::string(argv[++i]));
        }

        if (strcmp(argv[i], "-port") == 0) {
            sscanf(argv[++i], "%i", &udpPort);
        }
    }

    Router* routerCore = new Router(givenAddress ? Address::fromString(givenAddress) : Address::generateRandom(), Location(0,0));

#ifdef __APPLE__
    TunnelDeliveryInterface_Apple *tunIface = new TunnelDeliveryInterface_Apple(routerCore->getLocalIface(),
                                                                                routerCore->getAddress());
#endif
#ifdef __linux__
    TunnelDeliveryInterface_Linux *tunIface = new TunnelDeliveryInterface_Linux(routerCore->getLocalIface(),
                                                                                routerCore->getAddress());
#endif

    tunIface->startTunnelInterface();

    UDPManager *udpMan = new UDPManager(routerCore->getLinkManager(), udpPort);

    for (std::string& str : peers) {

        // TODO distinguish between ipv4 and ipv6 addresses
        // and do some checking
        std::string address = str.substr(0,str.find(":"));

        int port = std::stoi(str.substr(str.find(":")));

        udpMan->connectTo(address, port);
    }

    while (1) {
        udpMan->pollMessages();
        tunIface->pollMessages();

        usleep(1000000); // Sleep 0.01 seconds
    }

}