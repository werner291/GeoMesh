/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include "Scheduler.hpp"

void Scheduler::scheduleTask(const Task& task) {
        if (updating) {
            toAdd.push_back(task);
        } else {
            tasks.push_back(task);
        }
    }

void Scheduler::update() {

        assert(!updating); // Prevent updateception

        updating = true;

        time_point now = clock::now();

        duration delta = lastUpdate - now;

        for (auto itr = tasks.begin(); itr != tasks.end(); ++itr) {
            if (itr->planned <= now) {
                itr->callback(now, delta, *itr);

                if (itr->repeats) {
                    itr->planned += itr->interval;
                } else {
                    itr = tasks.erase(itr);
                }
            }
        }

        tasks.insert(tasks.end(), toAdd.begin(), toAdd.end());
        toAdd.clear();

        updating = false;
    }
