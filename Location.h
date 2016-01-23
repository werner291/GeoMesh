//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_LOCATION_H
#define MESHNETSIM_LOCATION_H


#include <string>
#include <sstream>

class Location {

public:
    double X;
    double Y;

    double distanceTo(const Location& other) const;

    Location(double X, double Y) : X(X), Y(Y) {}

    const std::string getDescription() {
        std::stringstream ss;

        ss << "(" << X << "," << Y << ")";

        return ss.str();
    }
};




#endif //MESHNETSIM_LOCATION_H
