//
// Created by Werner Kroneman on 04-04-16.
//

#include "ContactsSet.hpp"

ContactsSet::const_iterator ContactsSet::findClosestEntry(const Address &query) const {

    auto a = entries.begin(); // Take an iterator to begin

    if (entries.empty()) {
        return entries.end();
    }

    auto b = entries.end() - 1; // And to end.

    int match = 0; // The bit position to test to check for prefix equality

    while (a < b) {

        int bitA = a->address.getBit(match);
        int bitB = b->address.getBit(match);

        if (bitA == 0 && bitB == 1) {
            // Find split point through binary search
            auto c = a;
            auto d = b;

            int dist;

            while ((dist = std::distance(c, d)) > 1) {

                auto middle = c + dist / 2; // Take average.

                if (middle->address.getBit(match) == 0) {
                    c = middle;
                } else { // Must be 1 by excluded middle
                    d = middle;
                }
            }

            if (query.getBit(match) == 0) {
                b = c;
            } else {
                a = d;
            }
        }
        match += 1;
    }

    return a;
}

ContactsSet::iterator ContactsSet::insert(const Entry &entry) {

    // http://stackoverflow.com/a/31294973/1582024

    if (entries.empty()) {
        return entries.insert(entries.end(), entry);
        // Otherwise we get some really wierd behavior with iterators moving before the begin.
    }

    auto first = entries.begin();
    auto last = entries.end() - 1;

    while (first < last) {

        bool equal = first > last;

        int distance = std::distance(first, last);
        auto mid = first + distance / 2;

        if (mid->address == entry.address) {
            *mid = entry;
            return mid;
        } else if (mid->address > entry.address) {
            last = mid - 1;
        } else {
            first = mid + 1;
        }
    }
    if (first == entries.end()) {
        return entries.insert(entries.end(), entry);
    } else if (first->address > entry.address) {
        return entries.insert(first, entry);
    } else if (first->address == entry.address) {
        *first = entry;
        return first;
    } else {
        return entries.insert(first + 1, entry);

        return first;
    }
}
