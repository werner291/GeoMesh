//
// Created by Werner Kroneman on 28-01-16.
//

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>

using namespace std;

#include "TunnelDeliveryInterface_Apple.h"

#include "../UniqueAddress.h"

#include "../Logger.h"

#include "UDPManager.h"

#include "../Router.h"


int main(int argc, char **argv) {

    Router* routerCore = new Router(Address::generateRandom(), Location(0,0));

    TunnelDeliveryInterface_Apple *tunIface = new TunnelDeliveryInterface_Apple(routerCore->getLocalIface(),
                                                                                routerCore->getAddress());

    tunIface->startTunnelInterface();

    UDPManager *udpMan = new UDPManager(routerCore->getLinkManager());

    while (1) {
        udpMan->pollMessages();
        tunIface->pollMessages();

        usleep(10000); // Sleep 0.01 seconds
    }

}