#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <random>
#include "NetworkSim.h"
#include "../Router.h"

#include "NetworkWidget.h"

NetworkWidget* nw;
NetworkSim* simulator;

int canSend;

void update(void*) {

    float simulation_speed = 0.2f;

    simulator->updateSimulation((1000.f/60.f)*simulation_speed);



    std::random_device rdev;
    std::mt19937 rgen(rdev());

    std::uniform_int_distribution<int> displacement(0, simulator->getNodes().size()-1);

    int node1 = displacement(rgen);
    int node2 = displacement(rgen);

    std::string message = "Hello!";

    if (canSend <= 0) {
        simulator->sendMessage(message, node1, node2);
        canSend = 50;
    } else {
        canSend--;
    }

    nw->redraw();

    Fl::repeat_timeout(1/60.f, update);
}

int main(int argc, char **argv) {

    simulator = new NetworkSim();

    simulator->createRandomNetwork(50, 700, 500);

    Fl_Window *window = new Fl_Window(800,600);
    Fl_Box *box = new Fl_Box(20,40,260,100,"Hello, World!");
    box->box(FL_UP_BOX);
    box->labelsize(36);
    box->labelfont(FL_BOLD+FL_ITALIC);
    box->labeltype(FL_SHADOW_LABEL);


    nw = new NetworkWidget(10, 10, 800, 600, "?", *simulator);
    window->resizable(nw);

    window->end();
    window->show(argc, argv);

    Fl::add_timeout(0.1, update);

    return Fl::run();
}