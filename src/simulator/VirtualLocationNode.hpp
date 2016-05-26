//
// Created by Werner Kroneman on 22-05-16.
//

#ifndef GEOMESH_VIRTUALLOCATIONNODE_HPP
#define GEOMESH_VIRTUALLOCATIONNODE_HPP

#include "../VirtualLocationManager.hpp"
#include <boost/container/flat_map.hpp>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkVersion.h>
#include <random>
#include <thread>
#include <iostream>
#include <vector>
#include "VirtualLocationNode.hpp"

const double bestDistance = 1;

template<typename T>
struct WeakPtrCompare {
    bool operator()(const std::weak_ptr<T>& a, const std::weak_ptr<T>& b) const
    {
        return a.lock().get() < b.lock().get();
    }
};

struct LocationNode {
    double x;
    double y;
    double z;

    double distanceTo(const LocationNode& other) const
    {
        double dx = x - other.x, dy = y - other.y, dz = z - other.z;
        return sqrt(dx*dx + dy*dy + dz * dz);
    }

    std::vector<std::weak_ptr<LocationNode> > directNeighbours;

    boost::container::flat_map<std::weak_ptr<LocationNode>,int, WeakPtrCompare<LocationNode> >
            indirectNeighbours;

    void updateLocation(double step);

    void getBFSNeighbours(LocationNode* parent,
                          int depth,
                          int maxDepth);

    void updateIndirectNeighbours();



};

#endif //GEOMESH_VIRTUALLOCATIONNODE_HPP
