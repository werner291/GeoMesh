//
// Created by Werner Kroneman on 27-01-16.
//

#include <limits.h>
#include <gtest/gtest.h>
#include "../src/Location.hpp"

const double EPSILON = 0.00000001;

TEST(location_unit_carthesian, northpole) {

    Vector3d conv = convertLocation(90,0);

    EXPECT_NEAR(conv.x, 0, EPSILON);
    EXPECT_NEAR(conv.y, 0, EPSILON);
    EXPECT_NEAR(conv.z, 1, EPSILON);
}

TEST(location_unit_carthesian, equator_zero) {

    Vector3d conv = convertLocation(0,0);

    EXPECT_NEAR(conv.x, 1, EPSILON);
    EXPECT_NEAR(conv.y, 0, EPSILON);
    EXPECT_NEAR(conv.z, 0, EPSILON);
}

TEST(location_unit_carthesian, equator_back) {

    Vector3d conv = convertLocation(180,0);

    EXPECT_NEAR(conv.x, -1, EPSILON);
    EXPECT_NEAR(conv.y, 0, EPSILON);
    EXPECT_NEAR(conv.z, 0, EPSILON);
}

TEST(location_distance, self_proximity) {

    Location a(0,0);

    EXPECT_NEAR(0, a.distanceTo(a), EPSILON);

}

TEST(location_distance, semicircle_equatorial) {

    Location a(0,0);

    Location b(180,0);

    EXPECT_NEAR(EARTH_RAD * M_PI, a.distanceTo(b), EPSILON);

}

TEST(location_distance, quartcircle_equatorial) {

    Location a(0,0);

    Location b(90,0);

    EXPECT_NEAR(EARTH_RAD * M_PI / 2, a.distanceTo(b), EPSILON);

}

TEST(location_distance, semicircle_polar) {

    Location a(0,90);

    Location b(0,-90);

    EXPECT_NEAR(EARTH_RAD * M_PI, a.distanceTo(b), EPSILON);

}

TEST(location_direction, north) {

    Location a(0,0);

    Location b(45,0);

    EXPECT_NEAR(a.getDirectionTo(b), 0, EPSILON);
}

TEST(location_direction, east) {

    Location a(0,0);

    Location b(0,10);

    EXPECT_NEAR(a.getDirectionTo(b), M_PI/2, EPSILON);
}

TEST(location_direction, west) {

    Location a(0,0);

    Location b(0,-15);

    EXPECT_NEAR(a.getDirectionTo(b), -M_PI/2, EPSILON);
}

TEST(location_direction, south) {

    Location a(0,0);

    Location b(-80,0);

    EXPECT_NEAR(std::abs(a.getDirectionTo(b)), M_PI, EPSILON);
}

TEST(location_int_angles, zero) {

    EXPECT_NEAR(degreesToIntAngle(0), 0, 10);

}

TEST(location_int_angles, angle_180) {

    EXPECT_NEAR(degreesToIntAngle(180), INT_MIN, 10);

}

TEST(location_int_angles, angle_minus_180) {

    EXPECT_NEAR(degreesToIntAngle(-180), -INT_MAX, 10);

}

TEST(location_int_angles, angle_360) {

    EXPECT_NEAR(degreesToIntAngle(270), -INT_MAX/2, 10);

}

TEST(location_int_angles, angle_90) {

    EXPECT_NEAR(degreesToIntAngle(90), INT_MAX/2, 10);

}

TEST(location_int_angles, angle_minus_90) {

    EXPECT_NEAR(degreesToIntAngle(-90), INT_MIN/2, 10);

}

TEST(location_int_angles, to_and_back_0) {

    EXPECT_NEAR(intAngleToDegrees(degreesToIntAngle(0)), 0, 10);

}

TEST(location_int_angles, to_and_back_minus_90) {

    EXPECT_NEAR(intAngleToDegrees(degreesToIntAngle(-90)), -90, 10);

}

TEST(location_serialisation, ser_deser) {

    Location loc(5, 10);

    uint8_t buffer[8];

    loc.toBytes(buffer);

    Location locDeser = Location::fromBytes(buffer);

    EXPECT_NEAR(loc.lat, locDeser.lat, 0.01);
    EXPECT_NEAR(loc.lon, locDeser.lon, 0.01);


}
