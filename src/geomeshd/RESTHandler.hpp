

#ifndef GEOMESH_RPC_H
#define GEOMESH_RPC_H

#include "json/json/json.h"

#include "HTTPServer.hpp"

/**
 * Represents any object in an encoding-agnostic manner.
 */
struct RESTObject {
    // A property tree describing the object
    Json::Value properties;
    // The path of the object, prepended with the resoure name.
    // For example if there is a resource named "ducks",
    // setting "location" to "donald" would yield the HTTP header
    // Location: /ducks/donald
    std::string location;
};

class RESTResource {

    public:
    /**
     * return a representation of the object in the request.
     * the resource name has been removed already.
     * for example, with get /ducks/donald http/1.1,
     * request would just be "donald".
     *
     * \return the restobject representing the object.
     *
     * \throws httpnotfoundexception if the object doesn't exist.
     * \throws HTTPMethoNotAllowedException if the method is not valid for this resource.
     */
    virtual RESTObject get(std::string request)
    {
        throw HTTPMethodNotAllowedException("get method not allowed.");
    }

    /**
     * create a new resource.
     *
     * \param request string of the group in which to create the object.
     *                if this is not a hierarchical collection, request must be empty.
     *
     * \param object  the client-specified representation of the object.
     *
     * \return the RESTObject representing the object including any default
     *     values assumed by the server. location must be set to the path
     *     where subsequent requests can find it.
     *
     * \throws httpnotfoundexception if the object doesn't exist.
     * \throws HTTPMethoNotAllowedException if the method is not valid for this resource.
     */
    virtual RESTObject post(std::string request, const RESTObject& object)
    {
        throw HTTPMethodNotAllowedException("post method not allowed.");
    }

    virtual RESTObject put(std::string request, const RESTObject& object)
    {
        throw HTTPMethodNotAllowedException("put method not allowed.");
    }

    virtual RESTObject del(std::string request)
    {
        throw HTTPMethodNotAllowedException("delete method not allowed.");
    }
};

/**
 * A HTTP request handler specifically focused on REST.
 *
 * Features a mapping from strings to Resource objects,
 * which represent encoding-agnostic sub-handlers.
 */
class RESTRequestHandler : public HTTPServer::RequestHandler
{

    std::map<std::string, RESTResource&> resources;

    public:

    /**
     * \pre !request.empty()
     */
    void handleRequest( HTTPResponse& out, HTTPRequest& request) override;
    
    void addResource(const std::string& name, RESTResource& resource)
    {
        resources.insert(std::pair<std::string, RESTResource&>(name,resource));
    }

};
#endif // GEOMESH_RPC_H

