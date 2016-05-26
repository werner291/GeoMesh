#include <vector>
#include <iostream>
#include <thread>
#include <random>

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointSource.h>
#include <vtkProperty.h>
#include <vtkLine.h>
#include <vtkDataSetMapper.h>
#include <vtkCommand.h>
#include <boost/container/flat_map.hpp>

#include "../VirtualLocationManager.hpp"
#include "vtkNetworkAdapter.hpp"

using namespace boost::container;

VirtualLocationNetwork network;

vtkSmartPointer<vtkPolyData> networkSource;

std::unique_ptr<vtkNetworkAdapter> networkAdapter;

class vtkTimerCallback : public vtkCommand
{
public:
    static vtkTimerCallback *New()
    {
        vtkTimerCallback *cb = new vtkTimerCallback;
        cb->TimerCount = 0;
        return cb;
    }

    virtual void Execute(vtkObject* caller, unsigned long eventId,
                         void *vtkNotUsed(callData))
    {
        for (int i = 0; i < 1; ++i)
        {
            network.update(0.1);
        }

        networkAdapter->updatePositions();

        vtkRenderWindowInteractor *iren =
                static_cast<vtkRenderWindowInteractor*>(caller);

        iren->Render();
    }

private:
    int TimerCount;

};

int main(int, char *[])
{
    ///////////////
    // VTK setup //
    ///////////////

    // Open a Window
    vtkSmartPointer<vtkRenderWindow> renderWindow =
            vtkSmartPointer<vtkRenderWindow>::New();

    renderWindow->SetSize(800,600);
    //renderWindow->FullScreenOn();

    // Interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    renderWindowInteractor->Initialize();

    // Create a renderer
    vtkSmartPointer<vtkRenderer> renderer =
            vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground (.3, .6, .3); // black background

    renderWindow->AddRenderer ( renderer );

    // Network visualisation



    networkSource = vtkSmartPointer<vtkPolyData>::New();

    networkAdapter.reset(new vtkNetworkAdapter(network, networkSource));

    vtkSmartPointer<vtkPolyDataMapper> networkMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    networkMapper->SetInputData(networkSource);

    vtkSmartPointer<vtkActor> networkActor =
            vtkSmartPointer<vtkActor>::New();
    networkActor->SetMapper(networkMapper);

    renderer->AddActor(networkActor);

    // Network generation
    network.generateCubicGridNetwork(50, 5, 2, 0, 0, 0, 0);
    networkAdapter->updateFull();

    // Sign up to receive TimerEvent
    vtkSmartPointer<vtkTimerCallback> cb =
            vtkSmartPointer<vtkTimerCallback>::New();
    renderWindowInteractor->AddObserver(vtkCommand::TimerEvent, cb);

    int timerId = renderWindowInteractor->CreateRepeatingTimer(100);

    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}