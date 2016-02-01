//
// Created by System Administrator on 2/1/16.
//

#include "UDPConnectionManager.h"

UDPConnectionManager::UDPConnectionManager(LinkManager* linkMgr) : linkMgr(linkMgr){

}

void UDPConnectionManager::startListening(int port) {



}

void UDPConnectionManager::connectTo(std::string address, int port) {

    UDPInterface* udpIface = new UDPInterface();

}