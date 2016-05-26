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

#include "../UniqueAddress.hpp"
#include "../GPSLocation.cpp"

#include "RESTResourceAdapters.hpp"

RESTObject ContactsResource::get(std::string request) 
{
    if (!request.empty() && request != "/")
        throw HTTPException(400,"Invalid query");

    RESTObject result;

    result.properties = Json::Value(Json::arrayValue);

    for (const ContactsSet::Entry& contact : contacts) 
    {
        Json::Value contactInfo(Json::objectValue);
        contactInfo["address"] = contact.address.toString();
        contactInfo["latitude"] = contact.location.lat;
        contactInfo["longitude"] = contact.location.lon;
        contactInfo["expires"] = (Json::UInt64) contact.expires;

        result.properties.append(contactInfo);
    }

    return result;
}

RESTObject ContactsResource::post(std::string request,
            const RESTObject& posted) 
{
    // An ID is not allowed on POST, it will be returned
    // in the Locaiton: header
    if (!request.empty() && request != "/")
        throw HTTPException(400,"Invalid query");

    
    Address addr;
    try {
        addr = Address::fromString(posted.properties["address"].asString());
    } catch (...) {
        // TODO Create a proper exception class
        throw HTTPException(400,"Invalid address");
    }

    GPSLocation loc(posted.properties["latitude"].asDouble(),
                 posted.properties["longitude"].asDouble());

    time_t expires = posted.properties["expires"].asUInt64();

    contacts.insert(ContactsSet::Entry(addr,loc,expires));

    // Return the object as required.
    RESTObject obj;
    obj.properties = posted.properties;
    // Set location to address
    obj.location = "/" + addr.toString();

    return obj;
}
