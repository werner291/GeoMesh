//
// Created by Werner Kroneman on 20-05-16.
//

#ifndef GEOMESH_INCOMINGHTTPREQUEST_H
#define GEOMESH_INCOMINGHTTPREQUEST_H

#include "HTTPStringUtils.hpp"
#include <string>
#include <unordered_map>
#include <sstream>

class HTTPServer;

class HTTPRequest
{
    friend class HTTPServer;
    // File descriptor for the socket.
    int clientSocket;
    // Whether what's currently being received is the header or not.
    enum
    {
        REQUEST, HEADER, BODY, DONE
    } state;
    // How many more bytes to receive before
    size_t expected_body;
    std::stringstream buffer;

    std::string request;
    std::string method;
    typedef std::unordered_map<std::string, std::string, CaseInsensitiveStringHasher, CaseInsensitiveStringIsEqual> HTTPHeaderMap;
    HTTPHeaderMap headers;

    void handleIncomingConnection();

public:

    HTTPRequest(int fd) {
    	state = REQUEST;
    	expected_body = 0;
    	clientSocket = fd;
    }

    const HTTPHeaderMap &getHeaders() const
    { return headers; }

    const std::string &getRequest()
    { return request; }

    const std::string &getMethod()
    { return method; }

    std::istream &getInputStream()
    { return buffer; }
};



#endif //GEOMESH_INCOMINGHTTPREQUEST_H
