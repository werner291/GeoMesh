//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_LOCATION_H
#define MESHNETSIM_LOCATION_H

#include <cmath>
#include <string>
#include <sstream>

#include "constants.h"

const double EARTH_RAD = 6371000; // Approx earth radius in meters

struct Vector3d {
    double x, y, z;
};

static int32_t inline degreesToIntAngle(double degrees) {
    degrees = degrees - floor((degrees + 180)/360)*360;
    return round(INT32_MAX * degrees / 180);
}

static double inline intAngleToDegrees(int32_t intAngle) {
    return intAngle * 180 / INT32_MAX;
}

class Location {

    double inline rad(double angle) const {
        return angle * M_PI / 180.0;
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

    static Location readFromPacket(int offset, DataBufferPtr packet);

    void writeToPacket(int offset, DataBufferPtr packet);
};

Vector3d convertLocation(const Location &loc);

Vector3d convertLocation(double lat, double lon);


#endif //MESHNETSIM_LOCATION_H