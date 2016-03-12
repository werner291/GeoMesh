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

#include <regex>


int main(int argc, char **argv) {

    char* givenAddress = NULL;

    std::vector< std::string > peers;

    int udpPort = 10976;

    for (int i=0; i < argc; i++) {
        // The caller wants to adopt a specific address
        if (strcmp(argv[i], "-ip6") == 0) {
            givenAddress = argv[++i];
        }

        // Add a UDP bridge peer (Ethernet peers are added automatically)
        if (strcmp(argv[i], "-peer") == 0) {
            peers.emplace_back(std::string(argv[++i]));
        }

        // Specify the port of the UDP bridge
        if (strcmp(argv[i], "-port") == 0) {
            sscanf(argv[++i], "%i", &udpPort);
        }
    }

    // Create a new router core
    Router* routerCore = new Router(givenAddress ? Address::fromString(givenAddress) : Address::generateRandom(), Location(0,0));

    // Start the tunnel interface (platform-specific)

#ifdef __APPLE__
    TunnelDeliveryInterface_Apple *tunIface = new TunnelDeliveryInterface_Apple(routerCore->getLocalIface(),
                                                                                routerCore->getAddress());
#endif
#ifdef __linux__
    TunnelDeliveryInterface_Linux *tunIface = new TunnelDeliveryInterface_Linux(routerCore->getLocalIface(),
                                                                                routerCore->getAddress());
#endif

    tunIface->startTunnelInterface();

    // Create a new UDP bridge manager
    UDPManager *udpMan = new UDPManager(routerCore->getLinkManager(), udpPort);

    // For all peers, send out a hello message
    for (std::string& str : peers) {

        // Extract address and port number from a x.x.x.x:port formatted address.
        std::regex reg("([0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}):([0-9]+)");
        std::smatch match;

        if (std::regex_search(str, match, reg) && match.size() > 1) {
            std::string address(match.str(1).c_str());
            int port = std::atoi(match.str(2).c_str());

            // Start connection (returns immediately after sending,
            // will retry automatically if no response (not implemented yet)
            udpMan->connectTo(address, port);
        } else {
            // Report formatting error
            Logger::log(LogLevel::WARN, "Invalid peer address: " + str);
        }
    }

    // For the whole duration of the program, poll for messages and handle time-related things.
    while (1) {
        udpMan->pollMessages();

        tunIface->pollMessages();

        usleep(10000); // Sleep 0.01 seconds = 10000 microseconds
    }

}