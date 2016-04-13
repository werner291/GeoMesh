//
// Created by Werner Kroneman on 04-04-16.
//

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
