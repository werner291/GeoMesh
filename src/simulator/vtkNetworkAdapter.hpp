//
// Created by Werner Kroneman on 22-05-16.
//

#ifndef GEOMESH_VTKNETWORKADAPTER_HPP
#define GEOMESH_VTKNETWORKADAPTER_HPP

#include <boost/container/flat_map.hpp>
#include <vtk-7.0/vtkSmartPointer.h>
#include <vtk-7.0/vtkPolyData.h>
#include "VirtualLocationNetwork.hpp"
#include "VirtualLocationNode.hpp"

class vtkNetworkAdapter
{

    VirtualLocationNetwork& network;
    vtkSmartPointer<vtkPolyData> polyData;

public:

    vtkNetworkAdapter(VirtualLocationNetwork& network,
                      vtkSmartPointer<vtkPolyData> polyData)
            : network(network), polyData(polyData)
    {}

    void updateFull();

    void updatePositions();
};


#endif //GEOMESH_VTKNETWORKADAPTER_HPP
