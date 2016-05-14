
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

#include "HTTPExceptions.hpp"

class HTTPServer;

class HTTPResponse {

    friend HTTPServer;

    int status = 200;
    std::string statusLine = "OK";
    std::map<std::string,std::string> headers;
    
    std::stringstream body;

    public:

    HTTPResponse(int status, const std::string& reason) {
        setStatus(status,reason);
    }

    void setStatus(int status, const std::string& reason) {
        this->status = status;
        this->statusLine = reason;
    }

    void setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    std::ostream& getBodyStream()
    { return body; }

    private:
    void writeTo(int fd) 
    {
        std::stringstream strstr;

        strstr << "HTTP/1.1 " << status << " " << statusLine << "\r\n";

        setHeader("Content-Length:", std::to_string(body.str().length()));

        for (std::pair<std::string,std::string> header : headers) {
            strstr << header.first << ": " << header.second << "\r\n";
        }

        strstr << "\r\n";

        write(fd, strstr.str().data(), strstr.str().length());

        write(fd, body.str().data(), body.str().length());
    }
};

class HTTPRequest
{
    friend HTTPServer;
    // File descriptor for the socket.
    int clientSocket;
    // Whether what's currently being received is the header or not.
    enum { REQUEST, HEADER, BODY, DONE } state;
    // How many more bytes to receive before 
    size_t expected_body;
    std::stringstream buffer;

    std::string request;
    std::string method;
    std::map<std::string, std::string> headers;

public:
    const std::map<std::string, std::string>& getHeaders() const 
    { return headers; }

    const std::string& getRequest()
    { return request; }

    const std::string& getMethod()
    { return method; }

    std::istream& getInputStream()
    { return buffer; }
};

class HTTPServer 
{
    int serverSocket;

    // A struct representing an onging request that may be
    // in multiple packets.
    std::list<HTTPRequest> activeRequests;

    public:
    class RequestHandler {
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
        virtual void handleRequest(HTTPResponse& out, HTTPRequest& request) = 0;
    };
    protected:

    RequestHandler& requestHandler;
    
    char* findSubstring(char* haystack, const int haystack_length,
            const char* needle, const int needle_length);
public:

    /**
     * Start the HTTP server. Will bind to specified TCP port.
     *
     * \param port The TCP port number to bind to.
     *
     * \param password The password to access the service.
     */
    HTTPServer(int port, std::string password, RequestHandler& requestHandler);

    void pollSocket();


    /**
     * Check whether the buffer holds enough bytes to
     * do something useful with the request.
     *
     * If not, this method has no effect.
     */
    void processIncomingBytes(HTTPRequest& conn);

    ~HTTPServer();
};
#endif // GEOMESH_HTTPSERVER_H

