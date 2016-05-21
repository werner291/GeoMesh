//
// Created by Werner Kroneman on 20-05-16.
//

#include "HTTPServer.hpp"
#include "HTTPResponse.hpp"

void HTTPResponse::writeTo(int fd)
{
    std::__1::stringstream strstr;

    strstr << "HTTP/1.1 " << status << " " << statusLine << "\r\n";

    setHeader("Content-Length:", std::__1::to_string(body.str().length()));

    for (std::__1::pair<std::__1::string, std::__1::string> header : headers)
    {
        strstr << header.first << ": " << header.second << "\r\n";
    }

    strstr << "\r\n";

    write(fd, strstr.str().data(), strstr.str().length());

    write(fd, body.str().data(), body.str().length());
}