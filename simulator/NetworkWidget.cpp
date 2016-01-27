//
// Created by Werner Kroneman on 22-01-16.
//

#include "NetworkWidget.h"

#include <iostream>


#include "../constants.h"
#include "../Router.h"
#include "../Logger.h"
#include "draw3d.h"

double t = 0;
int showConnections = 0;

struct Vector3d {
    double x, y, z;
};

Vector3d convertLocation(const Location &loc) {
    return Vector3d {
            cos(loc.lon * M_PI / 180) * cos(loc.lat * M_PI / 180),
            sin(loc.lon * M_PI / 180) * cos(loc.lat * M_PI / 180),
            sin(loc.lat * M_PI / 180)
    };
}

Location interpolateLocation(const Location &a, const Location &b, float t) {

    double aLon = a.lon;

    if (a.lon > b.lon + 180) {
        aLon -= 360;
    }

    return Location(a.lat * t + b.lat * (1 - t), aLon * t + b.lon * (1 - t));
}

void NetworkWidget::draw() {

    t += 0.01;

    if (!valid()) initOpenGl();

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    gluLookAt(cos(t / 5) * 2, sin(t / 5) * 2, 4, 0, 0, 0, 0, 0, 1);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHT0);

    float ambient[] = {0.1, 0.5, 0.5};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    glColor3f(1, 1, 1);

    //glutSolidSphere(0.99,32,32);



    glDisable(GL_LIGHTING);

    glPointSize(20);

    glBegin(GL_POINTS);

    for (auto node : netSim.getNodes()) {
        Vector3d nodePos = convertLocation(node->getLocation());

        glVertex3d(nodePos.x, nodePos.y, nodePos.z);
    }
    glEnd();


    for (const Link &link : netSim.getLinks()) {


        const Location a = link.a->getRouter()->getLocation();
        const Location b = link.b->getRouter()->getLocation();

        drawLineBetweenLocations(a, b);

    }


    for (const Link &link : netSim.getLinks()) {
        for (const SimulatedPacket &packet : link.packetsOnLine) {
            float t = packet.linkProgress / ((float) link.length + 0.01);

            if (packet.direction == SimulatedPacket::B) t = 1.f - t;

            Location intermediate = interpolateLocation(link.a->getRouter()->getLocation(),
                                                        link.b->getRouter()->getLocation(), t);

            Vector3d packetPos = convertLocation(intermediate);


            if (getPacketData<int32_t>(MESSAGE_TYPE, packet.data) == MSGTYPE_PAYLOAD) {
                double destinationX = getPacketData<double>(LOCATION_COORDINATE_X, packet.data);
                double destinationY = getPacketData<double>(LOCATION_COORDINATE_Y, packet.data);

                if (getPacketData<int32_t>(ROUTING_MODE, packet.data) == ROUTING_GREEDY) {
                    glColor3f(0.5, 1, 0.5);
                    glBegin(GL_POINTS);
                    glVertex3f(packetPos.x, packetPos.y, packetPos.z);
                    glEnd();
                } else {
                    glColor3f(1, 0, 0);
                    glBegin(GL_POINTS);
                    glVertex3f(packetPos.x, packetPos.y, packetPos.z);
                    glEnd();
                }
            } else if (getPacketData<int32_t>(MESSAGE_TYPE, packet.data) == MSGTYPE_PEERINFO) {
                glColor3f(0.5, 0, 1);
                glBegin(GL_POINTS);
                glVertex3f(packetPos.x, packetPos.y, packetPos.z);
                glEnd();

                double targetLon = getPacketData<double>(PEERINFO_LOCATION_LON, packet.data);
                double targetLat = getPacketData<double>(PEERINFO_LOCATION_LAT, packet.data);

                glColor3f(0, 0, 1);
                drawDirectLineBetweenLocations(intermediate, Location(targetLat, targetLon));
            }
        }
    }


    auto router = netSim.getNodes()[((showConnections++) / 25) % netSim.getNodes().size()];

    glColor3f(0, 0, 1);

    for (auto itr = router->getRoutingTable().begin(); itr != router->getRoutingTable().end(); itr++) {

        drawDirectLineBetweenLocations(router->getLocation(), itr->target);
    }

    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        Logger::log(LogLevel::ERROR, "OpenGL error: " + std::to_string(error));
    }


}

void NetworkWidget::drawLineBetweenLocations(const Location &a, const Location &b) const {
    return;
    int steps = a.distanceTo(b) * 5 / EARTH_RAD;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= steps; i++) {
        float t = i / (float) steps;

        Location intermediate = interpolateLocation(a, b, t);

        Vector3d pt = convertLocation(intermediate);

        glVertex3d(pt.x, pt.y, pt.z);
    }
    glEnd();
}

void NetworkWidget::drawDirectLineBetweenLocations(const Location &a, const Location &b) const {

    glBegin(GL_LINES);

    Vector3d aPos = convertLocation(a);

    glVertex3d(aPos.x, aPos.y, aPos.z);

    Vector3d bPos = convertLocation(b);

    glVertex3d(bPos.x, bPos.y, bPos.z);

    glEnd();
}

void NetworkWidget::initOpenGl() {

    configureViewport();


}

void NetworkWidget::configureViewport() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w(), h());

    float aspect = h() / w();

    gluPerspective(40, aspect, 0.1, 50);
}

