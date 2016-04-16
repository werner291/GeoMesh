//
// Created by Werner Kroneman on 06-04-16.
//

#ifndef GEOMESH_SCHEDULER_H
#define GEOMESH_SCHEDULER_H

#include <iostream>
#include <vector>
#include <chrono>
#include <functional>
#include <thread>
#include <assert.h>

class Scheduler {

public:

    struct Task;

    typedef std::chrono::system_clock::time_point time_point;
    typedef std::chrono::system_clock::duration duration;
    typedef std::function<void (time_point currentTime, duration delta, Task& task)> TaskCallback;

    struct Task {
        time_point planned;
        duration interval;
        bool repeats;

        TaskCallback callback;

        Task(time_point planned, duration interval, bool repeats, TaskCallback callback)
            : planned(planned), interval(interval), repeats(repeats), callback(callback) {

        }

        void cancel() {
            repeats = false;
        }
    };

private:

    std::vector<Task> tasks;

    time_point lastUpdate;

    // Some clients might want to start or stop tasks from the callback, make that safe to do.
    bool updating = false;
    std::vector<Task> toAdd;

    bool stop;

    std::thread* asyncThread;

public:

    Scheduler(bool async) {
	    stop = false;
        if (async) {
            asyncThread = new std::thread([&](){
                while (!stop) {
                    update();
                    usleep(100);
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
