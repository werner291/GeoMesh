//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_LOCATION_H
#define MESHNETSIM_LOCATION_H

#include <cmath>
#include <string>
#include <sstream>

const double EARTH_RAD = 6371000; // Approx earth radius in meters

class Location {

    double inline rad(double angle) const {
        return angle * 180 / M_PI;
    }

public:
    double lon; // From -180 to 180 degrees, with 0 at IERS Reference Meridian, + towards East
    double lat; // From -90 to 90 degrees, + towards North



    double distanceTo(const Location& other) const;

    Location(double lat, double lon) : lon(lon), lat(lat) { }

    const std::string getDescription() const {
        std::stringstream ss;

        ss << "(" << lat << "," << lon << ")";

        return ss.str();
    }

    const double getDirectionTo(const Location &target);

    const bool operator==(const Location& other) {
        return lon == other.lon && lat == other.lat;
    }
};




#endif //MESHNETSIM_LOCATION_H
