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
#include <queue>
#include <assert.h>
#include <mutex>
#include <condition_variable>

/**
 * Scheduler is a class responsible for deciding what happens
 * when on the scheduler thread.
 *
 * Client code can register tasks to be executed at specific times
 * and intervals, and these will be called when needed.
 *
 * You can also use the callAsap() method to execute the task
 * as soon as possible on the scheduler thread.
 *
 * Please note that, like event handlers, tasks should not take
 * longer than a millisecond or so, as one task executing means
 * other tasks cannot be executed.
 *
 * Usage:
 *    Call scheduleTask() to add a new task.
 * If using synchronously, the update() function must be called at a regular interval.
 * If using async, the callbacks will be called from another thread.
 *
 * Note: The scheduler checks for events to be executed every time update() is called.
 *       If it is called asynchronously, it is updated every 10 ms.
 */
class Scheduler {

public:

    typedef std::chrono::steady_clock clock;

    struct Task;

    typedef clock::time_point time_point;
    typedef clock::duration duration;
    typedef std::function<void (Task& task)> TaskCallback;

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

        bool operator<(const Task& other) const {
            return other.planned < planned;
        }
    };

private:

    std::priority_queue<Task, std::vector<Task> > tasks;

    // The last time when  update() was last called.
    time_point lastUpdate;

    // Set to false to make the background task stop
    volatile bool stop;

    std::unique_ptr<std::thread> asyncThread;

    /**
     * mutex to synchronize access to the task queue.
     */
    std::mutex tasksMux;

    std::condition_variable cv;

    bool isNextTaskDue();

    void runAsync();

public:

    Scheduler(bool async);

    ~Scheduler();

    void scheduleTask(const Task& task);

    /**
     * Schedule a task on the scheduler thread
     * as soon as possible.
     *
     * \param toCall function object to call on the
     *               Scheduler thread.
     *
     * \param immediateReturn
     *               Whether you want this function to return immediately,
     *               or when the task has been completed. Warning: calling
     *               without immediate return from the scheduler thread
     *               will cause a deadlock state!
     */
    void callAsap(std::function<void()> toCall, bool immediateReturn = true);

    void update();


};


#endif //GEOMESH_SCHEDULER_H
