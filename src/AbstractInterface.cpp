//
// Created by Werner Kroneman on 22-01-16.
//

#include "AbstractInterface.hpp"

uint16_t AbstractInterface::nextIfaceID = 1;

AbstractInterface::AbstractInterface() : iFaceID(nextIfaceID++) {
    assert(iFaceID != 0);
}
