//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_LOCATION_H
#define MESHNETSIM_LOCATION_H

#include <cmath>
#include <string>
#include <sstream>

class Location {

public:
    double X;
    double Y;

    double distanceTo(const Location& other) const;

    Location(double X, double Y) : X(X), Y(Y) {}

    const std::string getDescription() const {
        std::stringstream ss;

        ss << "(" << X << "," << Y << ")";

        return ss.str();
    }

    const double getDirectionTo(const Location& target) {
        return atan2(target.Y - Y, target.X - X);
    }

    const bool operator==(const Location& other) {
        return X == other.X && Y == other.Y;
    }
};




#endif //MESHNETSIM_LOCATION_H
