

#ifndef GEOMESH_HTTPEXCEPTION_H
#define GEOMESH_HTTPEXCEPTION_H

#include <exception>

class HTTPServer;

class HTTPException : std::runtime_error 
{
    std::string reason;
    int status;

public:
    HTTPException(int status, const std::string& reason)
        : std::runtime_error(std::to_string(status) + " " + reason),
          reason(reason), status(status) 
          
    { }

    const std::string& getReason()
    {
        return reason;
    }

    const int getStatus()
    {
        return status;
    }
};

class HTTPMethodNotAllowedException : public HTTPException 
{
    public:
        HTTPMethodNotAllowedException(const std::string& reason = "Method not allowed")
            : HTTPException(405, reason)
        {}
};

class HTTPNotFoundException : public HTTPException 
{
    public:
        HTTPNotFoundException(const std::string& reason = "Method not allowed")
            : HTTPException(404, reason)
        {}
};
#endif // GEOMESH_HTTPEXCEPTION_H

