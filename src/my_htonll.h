//
// Created by System Administrator on 3/12/16.
//

#ifndef GEOMESH_MY_HTONLL_H
#define GEOMESH_MY_HTONLL_H

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

#include "UniqueAddress.h"

#endif //GEOMESH_MY_HTONLL_H
