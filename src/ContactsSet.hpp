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

#ifndef GEOMESH_CONTACTSSET_H
#define GEOMESH_CONTACTSSET_H

#include "UniqueAddress.hpp"

#include "Location.hpp"

#include "my_htonll.hpp"

class ContactsSet {

public:
    struct Entry {

        Address address;
        Location location;
        time_t expires;

        const static size_t SERIALIZED_SIZE = ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE + 8;

        Entry(const Address& addr, const Location& loc, const time_t& expires) :
                address(addr), location(loc), expires(0) {}

        Entry(const uint8_t* bytes) :
                address(Address::fromBytes(bytes)),
                location(Location::fromBytes(bytes+ADDRESS_LENGTH_OCTETS))
        {
            expires = my_htonll(* reinterpret_cast<const uint64_t*>(bytes +
                    ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE));
        }

        void toBytes(uint8_t* buffer) {
            memcpy(buffer, address.getBytes(), ADDRESS_LENGTH_OCTETS);
            location.toBytes((uint8_t*)buffer + ADDRESS_LENGTH_OCTETS);
            *(reinterpret_cast<uint64_t*>(buffer + ADDRESS_LENGTH_OCTETS + 8)) = my_htonll(expires);
        }
    };

private:
    /*
     * Rep invariant: entries must always be sorted by address
     */
    std::vector<Entry> entries;

public:
    typedef std::vector<Entry>::iterator iterator;
    typedef std::vector<Entry>::const_iterator const_iterator;
    iterator begin() { return entries.begin(); }
    const_iterator begin() const { return entries.begin(); }
    iterator end() { return entries.end(); }
    const_iterator end() const { return entries.end(); }

    /**
     * Find the contact whose address most closely matches the query
     * using the xor distance metric.
     */
    const_iterator findClosestEntry(const Address& query) const;

    iterator insert(const Entry& entry);

    bool empty() const {
        return entries.empty();
    }

    size_t size() const {
	    return entries.size();
    }

    void recomputeSharedPrefix();

};


#endif //GEOMESH_CONTACTSSET_H
