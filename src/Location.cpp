//
// Created by Werner Kroneman on 22-01-16.
//

#include <cmath>
#include "Location.hpp"

// Haversine formula based on JavaScript implementation found on http://www.movable-type.co.uk/scripts/latlong.html
double Location::distanceTo(const Location& other) const {

    double lat1 = rad(lat);
    double lat2 = rad(other.lat);
    double deltaLat = lat2 - lat1;
    double deltaLon = rad(lon - other.lon);

    double a = sin(deltaLat / 2) * sin(deltaLat / 2) +
               cos(lat1) * cos(lat2) *
               sin(deltaLon / 2) * sin(deltaLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RAD * c;
}

// Warning: This is rally only a measure suitable for the face routing algorithm
// The only thing that is important is that the directional order is kept.
double Location::getDirectionTo(const Location &target) const {

    // Convert to carthesian coordinates, Z+ is the axis goign from the South to the North Pole
    // These don't need to be multiplied by the radius, since we just want the angle.
    // Thus, they already have length 1.

    // This is the current node's position
    Vector3d cartSelf = convertLocation(lat,lon);

    // Carthesian coordinates of the target's vector.
    Vector3d cartOther = convertLocation(target);

    // Take the cos of the angle between them
    double dot = cartSelf.x * cartOther.x + cartSelf.y * cartOther.y + cartSelf.z * cartOther.z;

    // Substract the projection of the target vector from the target vector, which results in
    // a projection onto the plane that the self position vector is a normal of.
    cartOther.x -= dot * cartSelf.x;
    cartOther.y -= dot * cartSelf.y;
    cartOther.z -= dot * cartSelf.z;

    // Take the projection's length
    double projLength = sqrt(cartOther.x * cartOther.x + cartOther.y * cartOther.y + cartOther.z * cartOther.z);

    // Normalize
    cartOther.x /= projLength;
    cartOther.y /= projLength;
    cartOther.z /= projLength;

    // Take a perpendicular by rotating 90 degrees (easy with spherical coordinates)
    // We can take any vector that is perpendicular, as long as it is consistent.
    Vector3d perp = convertLocation(lat + 90, lon);

    // Find the angle between the perpendicular to the normal and the other location
    double angle = acos(perp.x * cartOther.x + perp.y * cartOther.y + perp.z * cartOther.z);

    // Find the sign of that angle
    double sign = cartSelf.x * (cartOther.y * perp.z - cartOther.z * perp.y)
                  + cartSelf.y * (cartOther.z * perp.x - cartOther.x * perp.z)
                  + cartSelf.z * (cartOther.x * perp.y - cartOther.y * perp.x);

    if (sign > 0) {
        return angle;
    } else {
        return -angle;
    }


}

Vector3d convertLocation(const Location &loc) {
    return convertLocation(loc.lat,loc.lon);
}

Vector3d convertLocation(double lat, double lon) {

    Vector3d converted {
            cos(lon * M_PI / 180) * cos(lat * M_PI / 180),
            sin(lon * M_PI / 180) * cos(lat * M_PI / 180),
            sin(lat * M_PI / 180)
    };

    return converted;
}