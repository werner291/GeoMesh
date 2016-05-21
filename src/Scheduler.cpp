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

void Scheduler::scheduleTask(const Task& task)
{
    // Obtain a lock to avoid the background thread
    // interfering with us.
    // TODO check whether a lock_guard would be better here.
    tasksMux.lock();

    tasks.push(task);

    tasksMux.unlock();

    // If we're in async mode, wake up the background
    // thread so it starts processing stuff.
    cv.notify_one();
}

void Scheduler::callAsap(std::function<void()> toCall, bool immediateReturn)
{
    if (immediateReturn)
    {
        scheduleTask(Task(clock::now(), std::chrono::milliseconds(0), false, [&](Task& task) { toCall(); }));
    }
    else
    {

        // Here, we use a mutex to prevent us from leaving this
        // method until the task has been executed
        std::mutex mux;
        
        // Lock the mutex
        mux.lock();

        // Schedule the task immediately.
        scheduleTask(Task(clock::now(), std::chrono::milliseconds(0), false,
                [&](Task& task)
                {
                    toCall();

                    // Only unlock once the task has been completed.
                    mux.unlock();

                }
            ));

        // The thread will get stuck here until
        // the mutex was unlocked from the task.
        mux.lock();

        // Unlock it again immediately.
        mux.unlock();
    }
}

bool Scheduler::isNextTaskDue()
{
    tasksMux.lock();

    bool due = (!tasks.empty())
               && tasks.top().planned <= clock::now();

    tasksMux.unlock();

    return due;

}

void Scheduler::update()
{
    while (isNextTaskDue()) {
        // Lock since we're afecting the queue
        tasksMux.lock();

        //
        Task task = tasks.top();
        tasks.pop();

        tasksMux.unlock();

        task.callback(task);

        if (task.repeats) {
                task.planned += task.interval;
                tasksMux.lock();
                tasks.push(task);
                tasksMux.unlock();
            }
    }
}

Scheduler::Scheduler(bool async) {
    stop = false;
    if (async) {
        asyncThread.reset(
                new std::thread([&]()
          {
              runAsync();
          })
        );
    }
}

void Scheduler::runAsync()
{
    while (!stop)
    { // Run until stop requested

        // Obtain a lock on the tasks
        std::unique_lock<std::mutex> ulock(tasksMux);

        /**
         * The next block is designed to wait until
         * the task queue is non-empty and, if so,
         * until the first task in the queue is due.
         *
         * This is similar to
         *  cv.wait(ulock, isNextTaskDue())
         *
         * Except we need custom conditional timed wake-ups.
         *
         * About the control flow: when the wait stops,
         * tasks are executed after restarting the loop.
         *
         * Surious wakeups are also dealt with using
         * the if header guard and the outer while loop.
         */
        if ((tasks.empty())
               || tasks.top().planned > std::__1::chrono::steady_clock::now())
        {
            // Check whether we need to wait for any specific tasks
            // or whether we just need to wait until the queue is modified.
            bool tasksScheduled = ! tasks.empty();

            if (tasksScheduled)
            {
                // Get planned start time of first task
                time_point next_due = tasks.top().planned;

                cv.wait_until(ulock, next_due);
            }
            else
            {
                // No tasks are in the queue, we will be notified when
                // one arrives. Sleep indefinitely.
                cv.wait(ulock);
            }
        } else {

            //
            Task task = tasks.top();
            tasks.pop();

            tasksMux.unlock();

            task.callback(task);

            if (task.repeats) {
                task.planned += task.interval;
                tasksMux.lock();
                tasks.push(task);
                tasksMux.unlock();
            }
        }
    }
}

Scheduler::~Scheduler()
{
    stop = true;
    // If the background trhead is waiting, wake it up.
    cv.notify_all();
    if (asyncThread.get() != nullptr)
    {
        asyncThread->join();
    }
}