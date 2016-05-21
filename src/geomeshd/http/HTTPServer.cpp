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
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <functional>

#include "../../Logger.hpp"
#include "HTTPServer.hpp"
#include "../FileDescriptorNotifier.hpp"

using namespace std;

HTTPServer::HTTPServer(int port, std::string password,
		RequestHandler &requestHandler, FDNotifier &fdnotifier) :
		fdnotifier(fdnotifier), requestHandler(requestHandler) {
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
	if (b < 0)
		throw runtime_error("ERROR on binding");

	int flags = fcntl(serverSocket, F_GETFL, 0);
	fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

	listen(serverSocket, 5);

	fdnotifier.addFileDescriptor(serverSocket,
			std::bind(&HTTPServer::handleIncomingConnection, this, std::placeholders::_1));

}

void HTTPServer::handleIncomingConnection(int fd) {
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_length = sizeof(cli_addr);
	int clientSocket = ::accept(serverSocket, (struct sockaddr *) &cli_addr,
			&cli_addr_length);

	if (clientSocket >= 0) {

		activeRequests.emplace(std::piecewise_construct,
                        std::forward_as_tuple(fd),
                        std::forward_as_tuple(fd));

		Logger::log(LogLevel::DEBUG, "Acccepted RPC connection.");

		fdnotifier.addFileDescriptor(clientSocket,
				std::bind(&HTTPServer::handleConnectionData, this, std::placeholders::_1));
	} else {
		throw runtime_error(
				"Error while accepting connection: " + string(strerror(errno)));
	}
}

void HTTPServer::handleConnectionData(int fd) {

	auto itr = activeRequests.find(fd);

	if (itr == activeRequests.end()) {
		throw runtime_error(
				"Received data for non-existent request with fd "
						+ std::to_string(fd));
	}

	HTTPRequest &req = itr->second;

	int expected_read;
	ioctl(req.clientSocket,
			FIONREAD,
			&expected_read);

	Logger::log(LogLevel::DEBUG,
			"Received " + to_string(expected_read) + " bytes on RPC.");

	char tempBuffer[256];

	for (int byte = 0; byte < expected_read;) {
		int bytesRead = byte += read(req.clientSocket, tempBuffer, 256);

		if (bytesRead < 0) {
			throw runtime_error("Error while reading from socket.");
		}

		req.buffer.write(tempBuffer, bytesRead);
	}

	processIncomingBytes(req);

	if (req.state == HTTPRequest::DONE) {
		close(req.clientSocket);
		fdnotifier.removeFileDescriptor(req.clientSocket);
		activeRequests.erase(itr);
	}
}

/**
 * Check whether the buffer holds enough bytes to
 * do something useful with the request.
 *
 * If not, this method has no effect.
 */
void HTTPServer::processIncomingBytes(HTTPRequest &conn) {

	try {
		// Have we read the header yet?
		// If not, read it line-by-line
		while (conn.state == HTTPRequest::REQUEST
				|| conn.state == HTTPRequest::HEADER) {
			// If there is no CRLF in the buffer, keep waiting
			// for more data or a timeout.
			if (conn.buffer.str().find("\r\n") == string::npos)
				return; // No more lines.

			// Put it into an string
			string headerLine;

			std::getline(conn.buffer, headerLine, '\n');

			// HTTP specified CRLF, which doesn't work with getline
			// (or it does, platform-specific)
			// Just cut off the \r if it's there.
			if (headerLine.back() == '\r')
				headerLine.pop_back();

			if (conn.state == HTTPRequest::REQUEST) {
				// We read the request line, subsequent lines will
				// be header fields or the header/body separation line.
				conn.state = HTTPRequest::HEADER;

				// Find the first space seperating the method and the request
				auto firstSpace = headerLine.find(" ");

				if (firstSpace == string::npos) {
					// If absent, tell the client and close.
					HTTPResponse(400, "Bad request").writeTo(conn.clientSocket);
					conn.state = HTTPRequest::DONE;
				}

				// Find the second space
				auto secondSpace = headerLine.find(" ", firstSpace + 1);

				if (secondSpace == string::npos) {
					HTTPResponse(400, "Bad request").writeTo(conn.clientSocket);
					conn.state = HTTPRequest::DONE;
				}

				// Set the method and request in the connection
				conn.method = headerLine.substr(0, firstSpace);
				conn.request = headerLine.substr(firstSpace + 1,
						secondSpace - firstSpace - 1);

				// TODO deal with different protocols.
				string protocol = headerLine.substr(secondSpace);
			} else {
				// Parse header lines
				if (headerLine.length() == 0) {
					// An empty line indicates the end of the header
					conn.state = HTTPRequest::BODY;

					auto itr = conn.headers.find("Content-Length");

					if (conn.getMethod() != "GET") {
						if (itr == conn.headers.end()) {
							throw HTTPException(400,
									"Missing Content-Length header.");
						} else {
							std::cout << itr->first << ": " << itr->second
									<< std::endl;
							conn.expected_body = std::stoi(itr->second);
						}
					}
				} else {
					auto colonPos = headerLine.find(":");
					if (colonPos == string::npos)
						throw HTTPException(400,
								"Invalid header, missing colon");

					// Find the first non-whitespace char after the :
					auto valuePos = headerLine.find_first_not_of(" ",
							colonPos + 1);
					if (valuePos == string::npos)
						throw HTTPException(400,
								"Invalid header, missing value");

					// Find the last non-whitespace character
					auto valueEnd = headerLine.length() - 1;
					while (headerLine[valueEnd] == ' ')
						--valueEnd;

					string key = headerLine.substr(0, colonPos);
					string value = headerLine.substr(colonPos + 1,
							valueEnd - colonPos - 1);

					conn.headers[key] = value;
				}
			}
		}

		std::cout << "Received: " << conn.buffer.tellp() - conn.buffer.tellg()
				<< " of " << conn.expected_body << std::endl;

		// The header was passed, is the body as large as we expect yet?
		assert(conn.buffer.tellp() >= conn.buffer.tellg());
		size_t bufLength = conn.buffer.tellp() - conn.buffer.tellg();

		if (bufLength >= conn.expected_body) {
			// Yes! If the request was correct, we should be able to
			// handle it now! Set status to 200 for now.
			HTTPResponse response(200, "OK");

			requestHandler.handleRequest(response, conn);

			response.writeTo(conn.clientSocket);

			conn.state = HTTPRequest::DONE;
		}
	} catch (HTTPException ex) {
		HTTPResponse(ex.getStatus(), ex.getReason()).writeTo(conn.clientSocket);

		conn.state = HTTPRequest::DONE;
	}
}

HTTPServer::~HTTPServer() {
	close(serverSocket);
}

