//
// Created by Werner Kroneman on 22-05-16.
//

#ifndef GEOMESH_VIRTUALLOCATIONNETWORK_HPP
#define GEOMESH_VIRTUALLOCATIONNETWORK_HPP

#include "VirtualLocationNode.hpp"


class VirtualLocationNetwork
{

    std::vector<std::shared_ptr<LocationNode> > nodes;

    bool dirty = true;

public:
    std::vector<std::shared_ptr<LocationNode> >& getNodes()
    {
        return nodes;
    }

    void update(double step);

    void connectNodes(std::weak_ptr<LocationNode> a,
                      std::weak_ptr<LocationNode> b);

    void generateCubicGridNetwork(const int gridSizeX,
                                      const int gridSizeY,
                                      const int gridSizeZ,
                                      const int positonRandomness,
                                      double centerX,
                                      double centerY,
                                      double centerZ);

};


#endif //GEOMESH_VIRTUALLOCATIONNETWORK_HPP
