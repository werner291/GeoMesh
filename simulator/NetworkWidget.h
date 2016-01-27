//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_NETWORKWIDGET_H
#define MESHNETSIM_NETWORKWIDGET_H

#include <FL/Fl_Widget.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/glut.h>
#include <FL/glu.h>
#include <FL/gl.h>
#include "NetworkSim.h"
#include "../Location.h"

class NetworkWidget : public Fl_Gl_Window {

    NetworkSim& netSim;


private:
    float scale = 1;

public:
    NetworkWidget(int x, int y, int w, int h, char const *label, NetworkSim &netSim) : Fl_Gl_Window(x, y, w, h, label),
                                                                                       netSim(netSim) { }

    float getScale() const {
        return scale;
    }

    void setScale(float scale) {
        NetworkWidget::scale = scale;
    }

private:
    void draw();

    void initOpenGl();

    void configureViewport();

    void drawLineBetweenLocations(const Location &a, const Location &b) const;

    void drawDirectLineBetweenLocations(const Location &a, const Location &b) const;
};


#endif //MESHNETSIM_NETWORKWIDGET_H
