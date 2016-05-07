//
// Created by Werner Kroneman on 22-01-16.
//

#include "AbstractLinkEndpoint.hpp"

uint16_t AbstractLinkEndpoint::nextIfaceID = 1;

AbstractLinkEndpoint::AbstractLinkEndpoint() : iFaceID(nextIfaceID++) {
    assert(iFaceID != 0);
}
