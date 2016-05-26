//
// Created by Werner Kroneman on 22-05-16.
//

#include <queue>
#include "VirtualLocationNode.hpp"

void LocationNode::updateLocation(double step)
{

    double fx = 0;
    double fy = 0;
    double fz = 0;

    for (auto neighbour_pair : indirectNeighbours)
    {
        std::shared_ptr<LocationNode> neighbour = neighbour_pair.first.lock();

        //if (neighbour_pair.second > 1) continue;
        double dx = neighbour->x - x;
        double dy = neighbour->y - y;
        double dz = neighbour->z - z;

        double vecLength = sqrt(dx * dx + dy * dy + dz * dz);

        if (vecLength > 0)
        {

            // Normalize
            dx /= vecLength;
            dy /= vecLength;
            dz /= vecLength;

            double dist = vecLength - bestDistance * neighbour_pair.second;

            if (neighbour_pair.second <= 2)
            {
                fx += (dx) * dist;
                fy += (dy) * dist;
                fz += (dz) * dist;
            }
        }
    }

    double dist = sqrt(fx * fx + fx * fx + fx * fx);

    if (dist > bestDistance)
    {
        x += fx * bestDistance / dist;
        y += fy * bestDistance / dist;
        z += fz * bestDistance / dist;

    }
    else
    {
        x += fx * dist;
        y += fy * dist;
        z += fz * dist;
    }
}


void LocationNode::updateIndirectNeighbours()
{
    indirectNeighbours.clear();

    std::queue<std::weak_ptr<LocationNode> > bfsQueue;

    for (std::weak_ptr<LocationNode>& neighbour : directNeighbours)
    {
        bfsQueue.push(neighbour);
    }

    int depth = 1;
    int maxDepth = 0;
    int nextDepthIn = directNeighbours.size();

    while (!bfsQueue.empty() && depth <= nextDepthIn)
    {
        auto neighbour = bfsQueue.front();
        bfsQueue.pop();


        if (indirectNeighbours.find(neighbour) == indirectNeighbours.end()
            && neighbour.lock().get() != this)
        {
            indirectNeighbours.insert(make_pair(neighbour, depth));

            for (std::weak_ptr<LocationNode>& neighbor :
                    neighbour.lock()->directNeighbours)
            {
                bfsQueue.push(neighbor);
            }
        }

        if (--nextDepthIn == 0)
        {
            nextDepthIn = bfsQueue.size();
            depth += 1;
        }
    }


}