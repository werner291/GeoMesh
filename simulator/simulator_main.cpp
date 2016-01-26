#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <random>
#include "NetworkSim.h"
#include "../Router.h"

#include "NetworkWidget.h"

NetworkWidget* nw;
NetworkSim* simulator;

int canSend;

void update(void*) {

    float simulation_speed = 0.5f;

    simulator->updateSimulation((1000.f*simulation_speed/60.f));

    std::random_device rdev;
    std::mt19937 rgen(rdev());

    std::uniform_int_distribution<int> displacement(0, simulator->getNodes().size()-1);

    int node1 = displacement(rgen);
    int node2 = displacement(rgen);

    std::string message = "Hello!";

    if (canSend <= 0) {
        simulator->sendMessage(message, node1, node2);
        canSend = 0;
    } else {
        canSend--;
    }

    nw->redraw();

    Fl::repeat_timeout(1/60.f, update);
}

int main(int argc, char **argv) {

    simulator = new NetworkSim();

    //simulator->createCrumpledGridNetwork(1100, 900);
    simulator->createIslandNetwork(1100, 900);

    std::unique_ptr< Fl_Window > window(new Fl_Window(1100, 900));

    std::unique_ptr< Fl_Box > box(new Fl_Box(900,0,200,900,"Hello, World!"));
    box->box(FL_UP_BOX);
    box->labelsize(36);
    box->labelfont(FL_BOLD+FL_ITALIC);
    box->labeltype(FL_SHADOW_LABEL);

    std::unique_ptr< Fl_Button > zoomIn(new Fl_Button(900,0,100,100,"+"));
    std::unique_ptr< Fl_Button > zoomOut(new Fl_Button(1000,0,100,100,"-"));

    nw = new NetworkWidget(10, 10, 900, 900, "?", *simulator);
    window->resizable(nw);

    window->end();
    window->show(argc, argv);

    Fl::add_timeout(0.1, update);

    return Fl::run();
}