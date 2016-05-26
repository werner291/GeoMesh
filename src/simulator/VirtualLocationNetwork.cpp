//
// Created by Werner Kroneman on 22-05-16.
//

#include "VirtualLocationNetwork.hpp"

void VirtualLocationNetwork::update(double step)
{
    if (dirty)
    {
        for (std::shared_ptr<LocationNode> node : nodes)
        {
            node->updateIndirectNeighbours();
        }
        dirty = false;
    }

    for (std::shared_ptr<LocationNode> node : nodes)
    {
        node->updateLocation(step);
    }
}

void VirtualLocationNetwork::connectNodes(std::weak_ptr<LocationNode> a,
                                          std::weak_ptr<LocationNode> b)
{
    a.lock()->directNeighbours.push_back(b);
    b.lock()->directNeighbours.push_back(a);

    dirty = true;
}

void VirtualLocationNetwork::generateCubicGridNetwork(const int gridSizeX,
                                                      const int gridSizeY,
                                                      const int gridSizeZ,
                                                      const int positonRandomness,
                                                      double centerX,
                                                      double centerY,
                                                      double centerZ)
{
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(-positonRandomness,
                                                        positonRandomness
                                                       );

    int startIndex = nodes.size();

    int xBlockSize = gridSizeY * gridSizeZ;
    int yBlockSize = gridSizeZ;

    for (int x = 0; x < gridSizeX; ++x)
    {
        for (int y = 0; y < gridSizeY; ++y)
        {
            for (int z = 0; z < gridSizeZ; ++z)
            {
                std::shared_ptr<LocationNode> newNode =
                        std::make_shared<LocationNode>();

                nodes.push_back(newNode);

                newNode->x = centerX + x + distribution(generator);
                newNode->y = centerY + y + distribution(generator);
                newNode->z = centerZ + z + distribution(generator);

                if (x >= 1)
                {
                    int otherIndex = startIndex +
                            (x - 1) * xBlockSize + y * yBlockSize + z;
                    connectNodes(newNode,nodes[otherIndex]);
                }
                if (y >= 1)
                {
                    int otherIndex = startIndex +
                            x * xBlockSize + (y - 1) * yBlockSize + z;
                    connectNodes(newNode,nodes[otherIndex]);
                }
                if (z >= 1)
                {
                    int otherIndex = startIndex + x * xBlockSize + y *
                                                                         yBlockSize + (z - 1);
                    connectNodes(newNode,nodes[otherIndex]);
                }
            }
        }
    }
}