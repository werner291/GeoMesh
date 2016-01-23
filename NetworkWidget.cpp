//
// Created by Werner Kroneman on 22-01-16.
//

#include "NetworkWidget.h"

#include "Router.h"

#include <iostream>

#include "constants.h"

template<typename T> T getPacketData(int charposition, DataBufferPtr data) {
    return *(reinterpret_cast<T *>(data->data() + charposition));
}

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

            fl_color(FL_GREEN);
            fl_circle(packetPosX, packetPosY, 10);
            
            if (getPacketData<int32_t >(MESSAGE_TYPE,packet.data) == MSGTYPE_PAYLOAD) {
                double destinationX = getPacketData<double>(LOCATION_COORDINATE_X,packet.data);
                double destinationY = getPacketData<double>(LOCATION_COORDINATE_Y,packet.data);

                fl_color(FL_GRAY);
                fl_line(packetPosX,packetPosY,destinationX,destinationY);
            }
        }
    }

    fl_color(FL_BLACK);

    for (const std::shared_ptr<Router> node : netSim.getNodes()) {
        fl_circle(node->getLocation().X,node->getLocation().Y, 1);
        fl_circle(node->getLocation().X,node->getLocation().Y, 3);
        //fl_circle(node->getLocation().X,node->getLocation().Y, 5);
    }
}