//
// Created by Werner Kroneman on 28-01-16.
//

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>

using namespace std;

#include "TunnelInterface_Apple.h"


#if defined(__GLIBC__) && __GLIBC__ >=2 && __GLIBC_MINOR__ >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <sys/types.h>
#include <netinet/if_ether.h>
#endif

#include "UniqueAddress.h"

#include "Logger.h"



int main(int argc, char **argv) {

    Router* routerCore = new Router(Address::generateRandom(), Location(0,0));

    TunnelInterface_Apple* tunIface = new TunnelInterface_Apple();

    UDPConnectionManager* udpMan = new UDPConnectionManager(routerCore->getLinkManager());


}