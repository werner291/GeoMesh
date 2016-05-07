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

#ifndef MESHNETSIM_LOCATION_H
#define MESHNETSIM_LOCATION_H

#include <cmath>
#include <string>
#include <sstream>
#include <arpa/inet.h>
#include <memory>

const double EARTH_RAD = 6371000; // Approx earth radius in meters

struct Vector3d {
    double x, y, z;
};

static int32_t inline degreesToIntAngle(double degrees) {
    degrees = degrees - floor((degrees + 180)/360)*360;
    return round(INT32_MAX * degrees / 180);
}

static double inline intAngleToDegrees(int32_t intAngle) {
    return intAngle * 180.0 / (double) INT32_MAX;
}

/**
 * A class representing a specific location somewhere on Earth.
 *
 * Should sending to different planets be necessary, please open an issue on GitHub.
 */
class Location {

    /**
     * Convert degrees to radians.
     */
    static double inline rad(double angle) {
        return angle * M_PI / 180.0;
    }

public:

    const static size_t SERIALIZED_SIZE = 8;

    double lon; // From -180 to 180 degrees, with 0 at IERS Reference Meridian, + towards East
    double lat; // From -90 to 90 degrees, + towards North

    double distanceTo(const Location& other) const;

    Location(double lat, double lon) : lon(lon), lat(lat) { }

    /**
     * Convert this location infto an std::string representation
     * for debugging purposed. (lat,lon).
     */
    const std::string getDescription() const {
        std::stringstream ss;

        ss << "(" << lat << "," << lon << ")";

        return ss.str();
    }

    /**
     * Get a direction to the target location.
     *
     * The only guarantee is that the angle will be bewteen -PI and PI,
     * and the values returned will be consistent when performed by the same node.
     *
     * This is useful when you need to know how locations are ordered when arranged on a compass.
     */
    double getDirectionTo(const Location &target) const;

    inline bool operator==(const Location &other) const {
        return lon == other.lon && lat == other.lat;
    }

    /**
     * Extract a Location from 16 octets of binary information.
     */
    inline static Location fromBytes(const uint8_t *buffer) {

        uint32_t latInt = ntohl(((uint32_t *) buffer)[0]);
        uint32_t lonInt = ntohl(((uint32_t *) buffer)[1]);

        return Location(
                intAngleToDegrees(latInt),
                intAngleToDegrees(lonInt)
        );

    }

    /**
     * Write this location into the buffer.
     * 16 bytes must be available in the buffer, and will be overwritten.
     */
    inline void toBytes(const uint8_t *buffer) const {

        uint32_t latInt = degreesToIntAngle(lat);
        uint32_t lonInt = degreesToIntAngle(lon);

        ((uint32_t *) buffer)[0] = htonl(latInt);
        ((uint32_t *) buffer)[1] = htonl(lonInt);

    }
};

/**
 * Convert a Location (which is in spherical coordinates) to a Euclidian represenentation.
 * This vector is of length 1, and does not take Earth's radius into account.
 *
 * (Lat = 0, Lon = 0) = (1,0,0)
 * (Lat = 90, Lon = any) = (0,0,1)
 * (Lat = 0, Lon = 90) = (0,1,0)
 */
Vector3d convertLocation(const Location &loc);

/**
 * Same as convertLocation(const Location &loc),
 * but now without the latitude and longitude being packed into a Location object.
 */
Vector3d convertLocation(double lat, double lon);


#endif //MESHNETSIM_LOCATION_H
