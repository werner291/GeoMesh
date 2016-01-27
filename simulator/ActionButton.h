//
// Created by Werner Kroneman on 26-01-16.
//

#ifndef MESHNETSIM_ACTIONBUTTON_H
#define MESHNETSIM_ACTIONBUTTON_H

#include <Fl/Fl_Button.H>
#include <functional>

class ActionButton : public Fl_Button {

    std::function<void(ActionButton *source)> action;

    int handle(int event) override;

public:

    ActionButton(int xMin, int yMin, int width, int height, char *text) : Fl_Button(xMin, yMin, width, height, text) {

    };

    void setAction(const std::function<void(ActionButton *)> &action) {
        ActionButton::action = action;
    }


};


#endif //MESHNETSIM_ACTIONBUTTON_H
