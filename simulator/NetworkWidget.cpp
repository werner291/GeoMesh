//
// Created by Werner Kroneman on 22-01-16.
//

#include "NetworkWidget.h"

#include <iostream>

#include "../constants.h"
#include "../Router.h"

void NetworkWidget::draw() {
    fl_color(FL_WHITE);
    fl_rectf(0, 0, w(), h());


    for(const Link& link : netSim.getLinks()) {

        fl_color(FL_BLACK);
        fl_line(link.a->getRouter()->getLocation().X,link.a->getRouter()->getLocation().Y,
                link.b->getRouter()->getLocation().X,link.b->getRouter()->getLocation().Y);


        for(const SimulatedPacket& packet : link.packetsOnLine) {
            float t = packet.linkProgress / ((float) link.length + 0.01);

            if (packet.direction == SimulatedPacket::B) t = 1.f - t;
            
            double packetPosX = link.a->getRouter()->getLocation().X * t + link.b->getRouter()->getLocation().X * (1.f - t);

            double packetPosY = link.a->getRouter()->getLocation().Y * t + link.b->getRouter()->getLocation().Y * (1.f - t);

            if (getPacketData<int32_t >(MESSAGE_TYPE,packet.data) == MSGTYPE_PAYLOAD) {
                double destinationX = getPacketData<double>(LOCATION_COORDINATE_X,packet.data);
                double destinationY = getPacketData<double>(LOCATION_COORDINATE_Y,packet.data);

                fl_color(FL_GRAY);
                fl_line(packetPosX,packetPosY,destinationX,destinationY);

                if (getPacketData<int32_t >(ROUTING_MODE,packet.data) == ROUTING_GREEDY) {
                    fl_color(FL_GREEN);
                    fl_circle(packetPosX, packetPosY, 5);
                } else {
                    fl_color(FL_RED);
                    fl_circle(packetPosX, packetPosY, 5);
                    fl_color(fl_rgb_color(255,128,128));
                    double faceStartX = getPacketData<double >(ROUTING_FACE_START_X,packet.data);
                    double faceStartY = getPacketData<double >(ROUTING_FACE_START_Y,packet.data);
                    fl_circle(destinationX,destinationY,sqrt((faceStartX-destinationX)*(faceStartX-destinationX)+(faceStartY-destinationY)*(faceStartY-destinationY)));
                    fl_color(fl_rgb_color(128,128,255));
                    fl_circle(destinationX,destinationY, getPacketData<double >(ROUTING_FACE_RANGE,packet.data));

                    if (getPacketData<int32_t>(ROUTING_FACE_DIRECTION,packet.data) == FACE_ROUTE_RHL) {
                        fl_draw("R",packetPosX,packetPosY+10);
                    } else {
                        fl_draw("L",packetPosX,packetPosY+10);
                    }
                }
            } else if (getPacketData<int32_t >(MESSAGE_TYPE,packet.data) == MSGTYPE_PEERINFO) {
                fl_color(fl_rgb_color(128,0,255));
                fl_circle(packetPosX, packetPosY, 5);
            }
        }
    }

    fl_color(FL_BLACK);

    fl_font(fl_font(), 10);

    for (const std::shared_ptr<Router> node : netSim.getNodes()) {
        fl_circle(node->getLocation().X,node->getLocation().Y, 1);
        fl_circle(node->getLocation().X,node->getLocation().Y, 3);
        fl_draw(node->getLocation().getDescription().c_str(),node->getLocation().X,node->getLocation().Y);
        fl_draw(std::to_string(node->getNumNeighbours()).c_str(),node->getLocation().X,node->getLocation().Y+10);
        //fl_circle(node->getLocation().X,node->getLocation().Y, 5);
    }
}