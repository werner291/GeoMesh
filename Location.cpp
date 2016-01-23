//
// Created by Werner Kroneman on 22-01-16.
//

#include <cmath>
#include "Location.h"

double Location::distanceTo(const Location& other) const {
    double deltaX = other.X - this->X;
    double deltaY = other.Y - this->Y;

    return sqrt(deltaX * deltaX + deltaY * deltaY);
}