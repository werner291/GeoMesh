//
// Created by Werner Kroneman on 28-01-16.
//

#ifndef GEOMESH_UDPINTERFACE_H
#define GEOMESH_UDPINTERFACE_H


class UDPInterface {

    UDPInterface();

    void connect(std::string address, int port);

    void pollMessages();

};


#endif //GEOMESH_UDPINTERFACE_H
