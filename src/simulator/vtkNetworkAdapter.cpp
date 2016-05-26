//
// Created by Werner Kroneman on 22-05-16.
//

#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkCellArray.h>

#include "vtkNetworkAdapter.hpp"

void vtkNetworkAdapter::updateFull()
{
    vtkSmartPointer<vtkPoints> points =
            vtkSmartPointer<vtkPoints>::New();

    vtkSmartPointer<vtkCellArray> verts =
            vtkSmartPointer<vtkCellArray>::New();

    vtkSmartPointer<vtkCellArray> lines =
            vtkSmartPointer<vtkCellArray>::New();

    auto nodes = network.getNodes();

    boost::container::flat_map<std::shared_ptr<LocationNode>, vtkIdType>
            nodePts;

    for (std::shared_ptr<LocationNode> nodePtr : nodes)
    {
        nodePts.insert(std::make_pair(nodePtr, points
                ->InsertNextPoint(nodePtr->x, nodePtr->y, nodePtr->z)));
    }

    for (std::shared_ptr<LocationNode> nodePtr : nodes)
    {
        vtkIdType idA = nodePts[nodePtr];
        for (std::weak_ptr<LocationNode> neighbourPtr : nodePtr->directNeighbours) {
            vtkIdType idB = nodePts[neighbourPtr.lock()];

            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, idA);
            line->GetPointIds()->SetId(1, idB);
            lines->InsertNextCell(line);
        }
    }

    polyData->SetPoints(points);
    polyData->SetVerts(verts);
    polyData->SetLines(lines);

    polyData->Modified();
}

void vtkNetworkAdapter::updatePositions()
{

    auto nodes = network.getNodes();

    auto points = polyData->GetPoints();

    for (int index = 0; index < nodes.size(); ++index)
    {
        const auto nodePtr = nodes[index];
        points->SetPoint(index, nodePtr->x, nodePtr->y, nodePtr->z);
    }

    polyData->SetPoints(points);

    polyData->Modified();
}