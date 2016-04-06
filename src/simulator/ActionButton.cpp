//
// Created by Werner Kroneman on 26-01-16.
//

#include "ActionButton.hpp"

int ActionButton::handle(int event) {

    if (event == FL_RELEASE) {
        action(this);
        return 1;
    } else {
        return 0;
    }


}