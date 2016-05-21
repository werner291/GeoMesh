//
// Created by Werner Kroneman on 20-05-16.
//

#ifndef GEOMESH_HTTPRESPONSE_HPP
#define GEOMESH_HTTPRESPONSE_HPP

#include "HTTPStringUtils.hpp"

class HTTPServer;

class HTTPResponse
{

    friend class HTTPServer;

    int status = 200;
    std::string statusLine = "OK";
    std::unordered_map<std::string, std::string, CaseInsensitiveStringHasher, CaseInsensitiveStringIsEqual> headers;

    std::stringstream body;

public:

    HTTPResponse(int status, const std::string &reason)
    {
        setStatus(status, reason);
    }

    void setStatus(int status, const std::string &reason)
    {
        this->status = status;
        this->statusLine = reason;
    }

    void setHeader(const std::string &key, const std::string &value)
    {
        headers[key] = value;
    }

    std::ostream &getBodyStream()
    { return body; }

private:
    void writeTo(int fd);
};

#endif //GEOMESH_HTTPRESPONSE_HPP
