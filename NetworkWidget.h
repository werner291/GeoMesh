//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_NETWORKWIDGET_H
#define MESHNETSIM_NETWORKWIDGET_H

#include <FL/FL_Widget.h>
#include <FL/fl_draw.H>
#include "NetworkSim.h"

class NetworkWidget : public Fl_Widget {

    NetworkSim& netSim;

public:
    NetworkWidget(int x, int y, int w, int h, char const *label, NetworkSim& netSim) : Fl_Widget(x, y, w, h, label), netSim(netSim) { }

private:
    void draw();
};


#endif //MESHNETSIM_NETWORKWIDGET_H
