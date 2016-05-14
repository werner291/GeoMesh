/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include "../Logger.hpp"
#include "HTTPServer.hpp"
#include <string.h>
#include <errno.h>
#include <assert.h>

using namespace std;

HTTPServer::HTTPServer(int port, string password, RequestHandler& requestHandler) 
    : requestHandler(requestHandler)
{
    struct sockaddr_in serv_addr;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0) 
        throw runtime_error("ERROR opening socket");
    
    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    
    int b = ::bind(serverSocket, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr));
    if (b< 0) 
        throw runtime_error("ERROR on binding");

    int flags = fcntl(serverSocket, F_GETFL, 0);
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

    listen(serverSocket,5);
    
}

void HTTPServer::pollSocket() 
{
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_length = sizeof(cli_addr);
    int clientSocket = ::accept(serverSocket,
            (struct sockaddr *) &cli_addr, &cli_addr_length);

    if (clientSocket >= 0) 
    {
        activeRequests.emplace_back();
        activeRequests.back().state = HTTPRequest::REQUEST;
        activeRequests.back().expected_body = 0;
        activeRequests.back().clientSocket = clientSocket;

        Logger::log(LogLevel::DEBUG, "Acccepted RPC connection.");
    }
    else if (errno != EWOULDBLOCK && errno != EAGAIN) 
    {
        throw runtime_error("Error while accepting connection: " 
                + string(strerror(errno)));
    }
    
    // Else, there are just no connections to accept.
    // Now, poll the connections we do have to see if there's any data.
    // TODO use the centralized select().
    for (auto itr = activeRequests.begin(); itr != activeRequests.end();) 
    {
        int count;
        ioctl(itr->clientSocket, FIONREAD, &count);

        try 
        {
            if (count > 0) 
            {
                
            Logger::log(LogLevel::DEBUG, "Received " + to_string(count) 
                    + " bytes on RPC.");

                int bytes = 256;
                char tempBuffer[256];

                // bytes != 256 indicates that the last read did not
                // fill the buffer, thus that we're out of bytes.
                // TODO is there a more explicit way?
                while (bytes == 256 && (bytes = read(itr->clientSocket, tempBuffer, 256)) > 0) {
                    itr->buffer.write(tempBuffer,bytes);
                    std::cout << "Read " << bytes << std::endl;
                }

                if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) 
                {
                    throw runtime_error("Error while reading from socket.");
                }

                processIncomingBytes(*itr);
            }

            if (itr->state == HTTPRequest::DONE) 
            {
                close(itr->clientSocket);
                itr = activeRequests.erase(itr);
            }
            else 
            {
                ++itr;
            }
        }
        catch (const exception& ex) 
        {
            Logger::log(LogLevel::ERROR,std::string("HTTPserver::pollSocket ") + ex.what());

                    close(itr->clientSocket);
                    itr = activeRequests.erase(itr);
        }
    }
}

/**
 * Check whether the buffer holds enough bytes to
 * do something useful with the request.
 *
 * If not, this method has no effect.
 */
void HTTPServer::processIncomingBytes(HTTPRequest& conn) 
{

    try {
        // Have we read the header yet?
        // If not, read it line-by-line
        while (conn.state == HTTPRequest::REQUEST 
                || conn.state == HTTPRequest::HEADER) 
        { 
            // If there is no CRLF in the buffer, keep waiting
            // for more data or a timeout.
            if (conn.buffer.str().find("\r\n") == string::npos) 
                return; // No more lines.

            // Put it into an string
            string headerLine;

            std::getline(conn.buffer,headerLine,'\n');

            // HTTP specified CRLF, which doesn't work with getline
            // (or it does, platform-specific)
            // Just cut off the \r if it's there.
            if (headerLine.back() == '\r')
                headerLine.pop_back();

            if (conn.state == HTTPRequest::REQUEST) 
            {
                // We read the request line, subsequent lines will
                // be header fields or the header/body separation line.
                conn.state = HTTPRequest::HEADER;

                // Find the first space seperating the method and the request
                auto firstSpace = headerLine.find(" ");

                if (firstSpace == string::npos) {
                    // If absent, tell the client and close.
                    HTTPResponse(400,"Bad request").writeTo(conn.clientSocket);
                    conn.state = HTTPRequest::DONE;
                }
                
                // Find the second space
                auto secondSpace = headerLine.find(" ", firstSpace+1);

                if (secondSpace == string::npos) {
                    HTTPResponse(400,"Bad request").writeTo(conn.clientSocket);
                    conn.state = HTTPRequest::DONE;
                }

                // Set the method and request in the connection
                conn.method = headerLine.substr(0,firstSpace);
                conn.request = headerLine.substr(firstSpace + 1, secondSpace 
                        - firstSpace - 1);
                
                // TODO deal with different protocols.
                string protocol = headerLine.substr(secondSpace);
            } 
            else 
            {
                // Parse header lines
                if (headerLine.length() == 0) 
                {
                    // An empty line indicates the end of the header
                    conn.state = HTTPRequest::BODY;

                    auto itr = conn.headers.find("Content-Length");

                    if (conn.getMethod() != "GET") {
                        if (itr == conn.headers.end()) {
                            throw HTTPException(400,"Missing Content-Length header.");
                        } else {
                            std::cout << itr->first << ": " << itr->second << std::endl;
                            conn.expected_body = std::stoi(itr->second);
                        }
                    }
                }
                else
                {
                    auto colonPos = headerLine.find(":");
                    if (colonPos == string::npos) 
                        throw HTTPException(400,"Invalid header, missing colon");
                    
                    // Find the first non-whitespace char after the :
                    auto valuePos = headerLine.find_first_not_of(" ", colonPos+1);
                    if (valuePos == string::npos) 
                        throw HTTPException(400,"Invalid header, missing value");
                    
                    // Find the last non-whitespace character
                    auto valueEnd = headerLine.length()-1;
                    while (headerLine[valueEnd] == ' ') --valueEnd;

                    string key = headerLine.substr(0,colonPos);
                    string value = headerLine.substr(colonPos+1, valueEnd-colonPos-1);

                    conn.headers[key] = value;
                }
            }
        }

        std::cout << "Received: " << conn.buffer.tellp() - conn.buffer.tellg()
           << " of " << conn.expected_body << std::endl; 

        // The header was passed, is the body as large as we expect yet?
        assert(conn.buffer.tellp() >= conn.buffer.tellg());
        size_t bufLength = conn.buffer.tellp() - conn.buffer.tellg();

        if (bufLength >= conn.expected_body) 
        {
            // Yes! If the request was correct, we should be able to
            // handle it now! Set status to 200 for now.
            HTTPResponse response(200,"OK");

            requestHandler.handleRequest(response, conn);

            response.writeTo(conn.clientSocket);

            conn.state = HTTPRequest::DONE;
        }
    } catch (HTTPException ex) {
        HTTPResponse(ex.getStatus(),ex.getReason()).writeTo(conn.clientSocket);

        conn.state = HTTPRequest::DONE;
    } 
}

HTTPServer::~HTTPServer() 
{
    close(serverSocket);
}
