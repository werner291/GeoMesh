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

/*
 * This file contains classes that make the bridge between
 * the RESTResource and the different components of GeoMesh.
 */

#ifndef GEOMESH_RESOURCE_ADAPTERS_H
#define GEOMESH_RESOURCE_ADAPTERS_H

#include "../ContactsSet.hpp"
#include "RESTHandler.hpp"

class ContactsResource : public RESTResource
{
    ContactsSet& contacts;

public:

    ContactsResource(ContactsSet& contacts)
        : contacts(contacts)
    { }

    RESTObject get(std::string request) override;

    RESTObject post(std::string request, const RESTObject& posted) override;

    virtual RESTObject put(std::string request, const RESTObject& object) override
    {
        throw HTTPMethodNotAllowedException("put method not allowed.");
    }

    virtual RESTObject del(std::string request) override
    {
        throw HTTPMethodNotAllowedException("delete method not allowed.");
    }
};




#endif // GEOMESH_RESOURCE_ADAPTERS_H
