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

#include "ContactsSet.hpp"

ContactsSet::const_iterator ContactsSet::findClosestEntry(const Address &query)
    const {

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
