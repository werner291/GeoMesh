#ifndef GEOMESH_HTTPSERVER_H
#define GEOMESH_HTTPSERVER_H

#include <map>
#include <list>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include <unordered_map>

#include "HTTPResponse.hpp"
#include "IncomingHTTPRequest.h"
#include "HTTPExceptions.hpp"
#include "../FileDescriptorNotifier.hpp"



class HTTPServer
{
    int serverSocket;

    FDNotifier& fdnotifier;

    // A struct representing an onging request that may be
    // in multiple packets.
    std::unordered_map<int, HTTPRequest> activeRequests;

public:
    class RequestHandler
    {
    public:
        /**
         * Override this method to handle incoming HTTP requests.
         *
         * You may also throw a HTTPException (and subclasses).
         * These will be caught, and the requester will receive
         * a short, pre-written message. You should only do this
         * for errors.
         *
         * \param out This object represents the HTTP response.
         *            After the method, the HTTPServer will use
         *            this object to generate the response
         *            for the client.
         *
         * \param request This object represents the request.
         *            The input stream starts reading from the
         *            start from the request body. The headers
         *            are accessible by seeking backwards,
         *            but you should use the getHeaders()
         *            instead.
         *
         *            You should not modify this object, except
         *            for reading from the input stream.
         */
        virtual void handleRequest(HTTPResponse &out, HTTPRequest &request) = 0;
    };

protected:

    RequestHandler &requestHandler;

    char *findSubstring(char *haystack, const int haystack_length,
                        const char *needle, const int needle_length);

public:

    /**
     * Start the HTTP server. Will bind to specified TCP port.
     *
     * \param port The TCP port number to bind to.
     *
     * \param password The password to access the service.
     */
    HTTPServer(int port, std::__1::string password, RequestHandler &requestHandler,
               FDNotifier &fdnotifier);

    void handleConnectionData(int fd);

    void pollSocket();

    void handleIncomingConnection(int fd);

    /**
     * Check whether the buffer holds enough bytes to
     * do something useful with the request.
     *
     * If not, this method has no effect.
     */
    void processIncomingBytes(HTTPRequest &conn);

    ~HTTPServer();
};

#endif // GEOMESH_HTTPSERVER_H

