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

#ifndef GEOMESH_SCHEDULER_H
#define GEOMESH_SCHEDULER_H

#include <iostream>
#include <vector>
#include <chrono>
#include <functional>
#include <thread>
#include <assert.h>

/**
 * Scheduler is a class responsible for timing-related functionality.
 *
 * Client code can register tasks to be executed at specific times
 * and intervals, and these will be called when needed.
 *
 * Usage:
 *    Call scheduleTask() to add a new task.
 * If using synchronously, the update() function must be called at a regular interval.
 * If using async, the callbacks will be called from another thread.
 */
class Scheduler {

public:

    typedef std::chrono::steady_clock clock;

    struct Task;

    typedef clock::time_point time_point;
    typedef clock::duration duration;
    typedef std::function<void (time_point currentTime, duration delta, Task& task)> TaskCallback;

    /**
     * Struct representing a task.
     */
    struct Task {
        // When to next execute the task. Task will be executed at first opportunity after time point has passed.
        time_point planned;

        // For repeating tasks, a duration after which to reschedule the task.
        duration interval;

        // Whether the task should be automatically re-scheduled after being run.
        bool repeats;

        // Callback to call when the planned time point has passed.
        TaskCallback callback;

        Task(time_point planned, duration interval, bool repeats, TaskCallback callback)
            : planned(planned), interval(interval), repeats(repeats), callback(callback) {

        }

        // For repeating tasks, call this method and it won't be repeated anymore.
        void cancel() {
            repeats = false;
        }
    };

private:

    std::vector<Task> tasks;

    // The last time when  update() was last called.
    time_point lastUpdate;

    // Some clients might want to start or stop tasks from the callback, make that safe to do.
    bool updating = false;
    std::vector<Task> toAdd;

    bool stop;

    std::thread* asyncThread;

public:

    /**
     * Create a new Scheduler.
     *
     * @param async Whether to start a thread in the background and call tasks
     *              automatically (please be careful about synchronisation).
     *              If not, you will have to call the update() function at a
     *              regular interval.
     */
    Scheduler(bool async) {
	    stop = false;
        if (async) {
            asyncThread = new std::thread([&](){
                std::this_thread::sleep_until(lastUpdate + std::chrono::milliseconds(10));
                while (!stop) {
                    update();
                    
                }
            });
        } else {
            asyncThread = nullptr;
        }
    }

    ~Scheduler() {
        stop = true;
        if (asyncThread != nullptr) {
            asyncThread->join();
            delete asyncThread;
        }
    }

    void scheduleTask(const Task& task); 

    void update();

};


#endif //GEOMESH_SCHEDULER_H
