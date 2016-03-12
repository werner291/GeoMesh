//
// Created by Werner Kroneman on 22-01-16.
//

#include "NetworkWidget.h"

#include <iostream>

#include "../Router.h"
#include "../Logger.h"
#include "draw3d.h"

double t = 0;
int showConnections = 0;

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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = h() / w();

    scale = 0.0011;

    gluPerspective(40, aspect, scale/2, scale * 50);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    Vector3d camPos = convertLocation(Location(51.4338784,5.4784266));

    gluLookAt(camPos.x * (1 + scale),camPos.y * (1 + scale),camPos.z * (1 + scale),
              0,0,0,
              0,0,1);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHT0);

    float ambient[] = {0.1, 0.5, 0.5};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    glColor3f(1, 1, 1);

    //glutSolidSphere(0.98,32,32);



    glDisable(GL_LIGHTING);

    glPointSize(5);

    glBegin(GL_POINTS);

    for (auto node : netSim.getNodes()) {
        Vector3d nodePos = convertLocation(node->getVirtualLocation());

        //glVertex3d(nodePos.x, nodePos.y, nodePos.z);
    }
    glEnd();


    for (const Link &link : netSim.getLinks()) {

        const Location a = link.a->getRouter()->getVirtualLocation();
        const Location b = link.b->getRouter()->getVirtualLocation();

        drawDirectLineBetweenLocations(a, b);

    }


    for (const Link &link : netSim.getLinks()) {
        for (const SimulatedPacket &packet : link.packetsOnLine) {
            float t = packet.linkProgress / ((float) link.length + 0.01);

            if (packet.direction == SimulatedPacket::B) t = 1.f - t;

            Location intermediate = interpolateLocation(link.a->getRouter()->getVirtualLocation(),
                                                        link.b->getRouter()->getVirtualLocation(), t);

            Vector3d packetPos = convertLocation(intermediate);


            if (getPacketData<int32_t>(MESSAGE_TYPE, packet.data) == MSGTYPE_PAYLOAD) {
                double destinationX = getPacketData<double>(LOCATION_COORDINATE_LON, packet.data);
                double destinationY = getPacketData<double>(LOCATION_COORDINATE_LAT, packet.data);

                if (getPacketData<int32_t>(ROUTING_MODE, packet.data) == ROUTING_GREEDY) {
                    glColor3f(0.5, 1, 0.5);

                    glBegin(GL_POINTS); glVertex3f(packetPos.x, packetPos.y, packetPos.z); glEnd();

                } else {
                    glColor3f(1, 0, 0);

                    glBegin(GL_POINTS); glVertex3f(packetPos.x, packetPos.y, packetPos.z); glEnd();

                    drawDirectLineBetweenLocations(intermediate, Location(destinationY, destinationX));

                }
            } else if (getPacketData<int32_t>(MESSAGE_TYPE, packet.data) == MSGTYPE_LOCATION_INFO) {
                glColor3f(0.5, 0, 1);
                glBegin(GL_POINTS);
                glVertex3f(packetPos.x, packetPos.y, packetPos.z);
                glEnd();

                double targetLon = getPacketData<double>(PEERINFO_LOCATION_LON, packet.data);
                double targetLat = getPacketData<double>(PEERINFO_LOCATION_LAT, packet.data);

                glColor3f(0, 0, 1);
                //drawDirectLineBetweenLocations(intermediate, Location(targetLat, targetLon));
            }
        }

    }


    auto router = netSim.getNodes()[((showConnections++) / 5) % netSim.getNodes().size()];

    glColor3f(0, 0, 1);

    for (auto itr = router->getRoutingTable().begin(); itr != router->getRoutingTable().end(); itr++) {

        drawDirectLineBetweenLocations(router->getVirtualLocation(), itr->target);
    }

    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        Logger::log(LogLevel::ERROR, "OpenGL error: " + std::to_string(error));
    }


}

void NetworkWidget::drawLineBetweenLocations(const Location &a, const Location &b) const {

    int steps = a.distanceTo(b) * 5 / EARTH_RAD + 1;

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

    glViewport(0, 0, w(), h());


}

