//
// Created by Werner Kroneman on 22-01-16.
//

#include <cmath>
#include "Location.h"

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
const double Location::getDirectionTo(const Location &target) {

    // Convert to carthesian coordinates, Z+ is the axis goign from the South to the North Pole
    // These don't need to be multiplied by the radius, since we just want the angle.
    // Thus, they already have length 1.

    // This is the current node's position
    double cartXself = cos(rad(lon)) * cos(rad(lat));
    double cartYself = sin(rad(lon)) * cos(rad(lat));
    double cartZself = sin(rad(lat));

    // Carthesian coordinates of the target's vector.
    double cartXother = cos(rad(target.lon)) * cos(rad(target.lat));
    double cartYother = sin(rad(target.lon)) * cos(rad(target.lat));
    double cartZother = sin(rad(target.lat));

    // Take the cos of the angle between them
    double dot = cartXself * cartXother + cartYself * cartYother + cartZself * cartZother;

    // Substract the projection of the target vector from the target vector, which results in
    // a projection onto the plane that the self position vector is a normal of.
    cartXother -= dot * cartXself;
    cartYother -= dot * cartYself;
    cartZother -= dot * cartZself;

    // Take the projection's length
    double projLength = sqrt(cartXother * cartXother + cartYother * cartYother + cartZother * cartZother);

    // Normalize
    cartXother /= projLength;
    cartYother /= projLength;
    cartZother /= projLength;

    // Find a perpendicular vector to compare the projection to.
    double perpX, perpY, perpZ;

    if (cartXself == 0 && cartYself == 0) { // Straight up, this is a special (and rather cold) case.
        perpX = 1;
        perpY = 0;
        perpZ = 0;
    } else {
        perpX = cartYself;
        perpY = -cartXself;
        perpZ = 0;

        double perpLength = perpX * perpX + perpY * perpY;

        perpX /= perpLength;
        perpY /= perpLength;  // Normalize
    }

    double angle = acos(perpX * cartXother + perpY * cartYother + perpZ * cartZother);

    double sign = perpX * (cartYother * perpZ - cartZother * perpY)
                  + perpY * (cartZother * perpX - cartXother * perpZ)
                  + perpZ * (cartXother * perpY - cartYother * perpZ);

    if (sign > 0) { // TODO check whether the sign should be reversed or not.
        return angle;
    } else {
        return -angle;
    }


}

