//
// Created by Werner Kroneman on 06-04-16.
//

#include <gtest/gtest.h>
#include "../src/Scheduler.hpp"

using namespace std::chrono;

TEST(scheduler_test, test_sync) {

    Scheduler sched(false);

    int calls = 0;

    sched.scheduleTask(Scheduler::Task(system_clock::now() + milliseconds(10), milliseconds(10), true,
                                       [&](Scheduler::time_point time, Scheduler::duration delta,
                                           Scheduler::Task &task) {
                                           calls++;

                                           ASSERT_TRUE(time - system_clock::now() < milliseconds(1)
                                                       && system_clock::now() - time < milliseconds(1));
                                       }));

    Scheduler::time_point start = system_clock::now();

    const int ITERATIONS = 500;

    for (int i = 0; i < ITERATIONS; ++i) {

        sched.update();

        std::this_thread::sleep_for(milliseconds(5));

    }

    Scheduler::duration elapsed = system_clock::now() - start;

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    EXPECT_NEAR(milliseconds / 10, calls, 1);
}

int calls = 0;

TEST(scheduler_test, test_async) {

    Scheduler sched(true);

    calls = 0;

    sched.scheduleTask(Scheduler::Task(system_clock::now() + milliseconds(10), milliseconds(10), true,
                                       [&](Scheduler::time_point time, Scheduler::duration delta,
                                           Scheduler::Task &task) {
                                           if (calls == -1) {
                                               task.repeats = false;
                                           } else {
                                               calls++;

                                               ASSERT_TRUE(time - system_clock::now() < milliseconds(1)
                                                           && system_clock::now() - time < milliseconds(1));
                                           }
                                       }));

    Scheduler::time_point start = system_clock::now();

    std::this_thread::sleep_for(milliseconds(5000));

    Scheduler::duration elapsed = system_clock::now() - start;

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    EXPECT_NEAR(milliseconds / 10, calls, 1);

    calls = -1;
}