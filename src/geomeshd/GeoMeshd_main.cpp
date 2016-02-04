//
// Created by Werner Kroneman on 28-01-16.
//

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>

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

    char* peerAddress = NULL;

    for (int i=0; i < argc; i++) {
        if (strcmp(argv[i], "-ip6") == 0) {
            givenAddress = argv[++i];
        }

        if (strcmp(argv[i], "-peer") == 0) {
            peerAddress = argv[++i];
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

    UDPManager *udpMan = new UDPManager(routerCore->getLinkManager());

    if (peerAddress) {
        udpMan->connectTo(peerAddress, 10976);
    }

    while (1) {
        udpMan->pollMessages();
        tunIface->pollMessages();

        usleep(10000); // Sleep 0.01 seconds
    }

}