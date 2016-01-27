#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <random>
#include "NetworkSim.h"
#include "../Router.h"
#include "ActionButton.h"

#include "NetworkWidget.h"

NetworkWidget* nw;
NetworkSim* simulator;

int canSend;

void update(void*) {

    float simulation_speed = 0.1f;

    simulator->updateSimulation((1000.f*simulation_speed/60.f));

    std::random_device rdev;
    std::mt19937 rgen(rdev());

    std::uniform_int_distribution<int> displacement(0, simulator->getNodes().size()-1);

    int node1 = displacement(rgen);
    int node2 = displacement(rgen);

    std::string message = "Hello!";

    if (canSend <= 0) {
        simulator->sendMessage(message, node1, node2);
        canSend = 10;
    } else {
        canSend--;
    }

    nw->redraw();

    Fl::repeat_timeout(1/60.f, update);
}

int main(int argc, char **argv) {

    simulator = new NetworkSim();

    simulator->createLongitudinalGridNetwork(15, 0);

    std::unique_ptr< Fl_Window > window(new Fl_Window(1100, 900));

    std::unique_ptr< Fl_Box > box(new Fl_Box(900,0,200,900,"Hello, World!"));
    box->box(FL_UP_BOX);
    box->labelsize(36);
    box->labelfont(FL_BOLD+FL_ITALIC);
    box->labeltype(FL_SHADOW_LABEL);


    nw = new NetworkWidget(0, 0, 900, 900, "?", *simulator);

    NetworkWidget *_nw = nw;

    window->resizable(_nw);

    std::unique_ptr<ActionButton> zoomIn(new ActionButton(900, 0, 100, 100, "+"));
    zoomIn->setAction([_nw](ActionButton *source) { nw->setScale(nw->getScale() * 1.1); });
    std::unique_ptr<ActionButton> zoomOut(new ActionButton(1000, 0, 100, 100, "-"));
    zoomOut->setAction([_nw](ActionButton *source) { nw->setScale(nw->getScale() / 1.1); });

    window->end();
    window->show(argc, argv);

    Fl::add_timeout(0.1, update);

    return Fl::run();
}