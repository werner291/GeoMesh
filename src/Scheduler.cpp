//
// Created by Werner Kroneman on 06-04-16.
//

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
