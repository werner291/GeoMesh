//
// Created by Werner Kroneman on 22-01-16.
//

#include "AbstractInterface.h"

int AbstractInterface::nextIfaceID = 0;

AbstractInterface::AbstractInterface() : iFaceID(nextIfaceID++) {

}