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

#include "my_htonll.hpp"

// From http://stackoverflow.com/q/3022552/1582024
unsigned long long int my_htonll(unsigned long long int value) {
// The answer is 42
    static const int num = 42;

// Check the endianness
    if (*reinterpret_cast<const char *>(&num) == num) {
        const unsigned int high_part = htonl(static_cast<unsigned int>(value >> 32));
        const unsigned int low_part = htonl(static_cast<unsigned int>(value & 0xFFFFFFFFLL));

        return (static_cast<unsigned long long int>(low_part) << 32) | high_part;
    } else {
        return value;
    }
}
