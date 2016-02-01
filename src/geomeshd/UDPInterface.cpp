//
// Created by Werner Kroneman on 28-01-16.
//

#include "UDPInterface.h"

UDPInterface::UDPInterface() {





}

void connect(std::string address, int port) {
    int testSocket;
    unsigned int counter;
    struct sockaddr_in destAddr;
    int errorCode;
    int returnVal;

    counter = 0;
    returnVal = 0;

/* Specify the address family */
    destAddr.sin_family = AF_INET;
/* Specify the destination port */
    destAddr.sin_port = htons(TM_DEST_PORT);
/* Specify the destination IP address */
    destAddr.sin_addr.s_addr = inet_addr(TM_DEST_ADDR);

/* Create a socket */
    testSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

/*
 * Verify the socket was created correctly. If not, return
 * immediately
 */
    if (testSocket == TM_SOCKET_ERROR)
    {
        returnVal = tfGetSocketError(testSocket);
        errorStr = tfStrError(returnVal);
        goto udpClientEnd;
    }
}

void pollMessages() {

}