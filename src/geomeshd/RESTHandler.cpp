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

#include <assert.h>

#include "json/json/json.h"

#include "RESTHandler.hpp"

using namespace std;

void RESTRequestHandler::handleRequest( HTTPResponse& out,
        HTTPRequest& request) 
{

    // Take a reference to the request string
    const std::string& req = request.getRequest();

    // Procondition check, the acceptor should have rejected
    // empty requests before handing them to this
    assert(!req.empty());

    // Must start with a /
    if (req[0] != '/')
        throw HTTPException(400, "Invalid request, must start with a /");

    // The resource is from the string from the first slash
    // until the second slash (or until the end if there
    // isn't any.
    std::string resource = (req.length() == 1) ? "" :
        req.substr(1,req.find("/",1));

    // Do we have a resource named like this?
    // If not, throw a 404
    auto itr = resources.find(resource);
    if (itr == resources.end()) 
        throw HTTPNotFoundException("Resource not found.");

    // The object name is the rest of the request
    // minus what we truncated earlier
    std::string objectName = req.substr(resource.length()+1);

    // The object to be given to the client
    RESTObject result;

    // Check whether the method is supported.
    if (request.getMethod() != "GET"
        && request.getMethod() != "POST"
        && request.getMethod() != "PUT"
        && request.getMethod() != "DELETE") {

        // TODO add the "Allowed:" header
        throw HTTPMethodNotAllowedException();
    }

    // Send the GET or DELETE to the resouce handler
    if (request.getMethod() == "GET")
    {
        result = itr->second.get(objectName);
    }
    else if (request.getMethod() == "DELETE") 
    {
        result = itr->second.del(objectName);
    }

    // POST and PUT requests have a body, parse and validate
    if (request.getMethod() == "POST" || request.getMethod() == "PUT") 
    {
        // The object provided by the client
        RESTObject obj;

        // Extract the Content-Type header
        // (or give a 400 if it isn't there)
        auto ctype = request.getHeaders().find("Content-Type");
        if (ctype == request.getHeaders().end())
            throw HTTPException(400, "Missing Content-Type header");
    
        // Decode the object
        try {
            if (ctype->second == "application/json")
                request.getInputStream() >> obj.properties;
            else
                throw HTTPException(415, "Unaccepted media type");
        } catch (Json::Exception& ex) {
            throw HTTPException(400, "Error while reading json");
        }

        // Now pass the request tothe put or post handler
        if (request.getMethod() == "POST")
        {
            result = itr->second.post(objectName,obj);
        }
        else if (request.getMethod() == "PUT")
        {
            result = itr->second.put(objectName, obj);
        }
    }

    // Check out in which format the client want the object
    auto accept = request.getHeaders().find("Accept");

    if (accept == request.getHeaders().end() 
            || accept->second.find("application/json") == string::npos)
        out.setStatus(406, "Not acceptable");
    
    // JSON (also if unspecified)

    Json::FastWriter writer;
    out.getBodyStream() << writer.write(result.properties);

    out.setHeader("Content-Type","application/json");

}
