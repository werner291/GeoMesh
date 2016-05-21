//
// Created by Werner Kroneman on 06-04-16.
//

#include <gtest/gtest.h>
#include "../src/Scheduler.hpp"

using namespace std::chrono;

TEST(scheduler_test, order_multiple_order)
{
    std::vector<Scheduler::Task> tasks;

    int order_checker = 0;

    Scheduler scheduler(false);

    for (int i = 0; i < 50; ++i)
    {

        tasks.emplace_back(Scheduler::clock::now() + milliseconds(i), milliseconds(0), false,
            [&order_checker,i](Scheduler::Task &task)
        {
            EXPECT_EQ(order_checker,i);
            order_checker += 1;
        });
    }

    std::random_shuffle(tasks.begin(), tasks.end());

    for (Scheduler::Task& task : tasks)
    {
        scheduler.scheduleTask(task);
    }

    Scheduler::time_point start = Scheduler::clock::now();

    do {

        std::this_thread::sleep_for(milliseconds(5));

        scheduler.update();

    } while (Scheduler::clock::now() < start + milliseconds(55) );

    EXPECT_EQ(50,order_checker);

}

TEST(scheduler_test, test_sync)
{

    Scheduler sched(false);

    int calls = 0;

    sched.scheduleTask(Scheduler::Task(Scheduler::clock::now() + milliseconds(10), milliseconds(10), true,
                                       [&](Scheduler::Task &task) {
                                           calls++;
                                       }));

    Scheduler::time_point start = Scheduler::clock::now();

    const int ITERATIONS = 500;

    for (int i = 0; i < ITERATIONS; ++i) {

        sched.update();

        std::this_thread::sleep_for(milliseconds(5));

    }

    Scheduler::duration elapsed = Scheduler::clock::now() - start;

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    EXPECT_NEAR(milliseconds / 10, calls, 1);
}

int calls = 0;

TEST(scheduler_test, test_async) {

    Scheduler sched(true);

    calls = 0;

    sched.scheduleTask(Scheduler::Task(Scheduler::clock::now() + milliseconds(10), milliseconds(10), true,
                                       [&](Scheduler::Task &task) {
                                           if (calls == -1) {
                                               task.repeats = false;
                                           } else {
                                               calls++;
                                           }
                                       }));

    Scheduler::time_point start = Scheduler::clock::now();

    std::this_thread::sleep_for(milliseconds(5000));

    Scheduler::duration elapsed = Scheduler::clock::now() - start;

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    EXPECT_NEAR(milliseconds / 10, calls, 1);

    calls = -1;
}

#if 0
/**
 * Test callASAP with immediate return, re-creating the scheduler each time
 *
 * Note: This test failed a few times, but I can't reproduce it
 * anymore. If anyone manages to reproduce, please tell me.
 */
TEST(scheduler_test, run_asap_immediate) {

    // Run 100 iterations to look for race conditions.
    int iterations = 100;

    for (int iter = 0; iter < iterations; ++iter)
    {
        Scheduler sched(true); // Async, run in background.

        if (rand() % 2 == 0)
        {
            // Once every two iterations, sleep for a bit
            // to see if it messes up the scheduler.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        testCallAsapReturnImmediate(sched);
    }
}


/**
 * Test callASAP with immediate return, while re-using the scheduler
 *
 * Note: This test failed a few times, but I can't reproduce it
 * anymore. If anyone manages to reproduce, please tell me.
 */
TEST(scheduler_test, run_asap_immediate) {

    // Run 100 iterations to look for race conditions.
    int iterations = 100;

    Scheduler sched(true); // Async, run in background.

    for (int iter = 0; iter < iterations; ++iter)
    {

        testCallAsapReturnImmediate(sched);
    }
}

/**
 * Test callASAP without immediate return, while re-creating
 * the scheduler object each time.
 *
 * Note: This test failed a few times, but I can't reproduce it
 * anymore. If anyone manages to reproduce, please tell me.
 */
TEST(scheduler_test, run_asap_wait) {

    // Run 100 iterations to look for race conditions.
    int iterations = 100;

    for (int iter = 0; iter < iterations; ++iter) {
        Scheduler sched(true); // Async, run in background.

        testCallAsapReturnWait(sched);
    }
}

/**
 * Test callASAP without immediate return, while re-using
 * the same scheduler object each time.
 *
 * Note: This test failed a few times, but I can't reproduce it
 * anymore. If anyone manages to reproduce, please tell me.
 */
TEST(scheduler_test, run_asap_wait) {

    // Run 100 iterations to look for race conditions.
    int iterations = 100;

    Scheduler sched(true); // Async, run in background.

    for (int iter = 0; iter < iterations; ++iter) {

        testCallAsapReturnWait(sched);
    }
}

////////////////////////////////////////////////////
// Helper methods and re-used parts of test cases //
////////////////////////////////////////////////////

/**
 * Test callAsap without immediate return.
 */
void testCallAsapReturnWait(Scheduler &sched) {

    if (rand() % 2 == 0) {
        // Once every two iterations, sleep for a bit
        // to see if it messes up the scheduler.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Record start time
    Scheduler::time_point start = Scheduler::clock::now();

    // Keep track of whether the background task was completed.
    volatile bool asyncExecuted = false;

    // Schedule the call WITHOUT IMMEDIATE RETURN
    sched.callAsap([&]() {
        // Sleep 50ms in Scheduler thread
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Set flag to true
        asyncExecuted = true;
    }, false);

    // About 50 ms should have passed
    EXPECT_LE(start + std::chrono::milliseconds(49), Scheduler::clock::now());

    // Task should have been executed.
    EXPECT_TRUE(asyncExecuted);
}

/**
 * Test callAsap with immediate return.
 */
void testCallAsapReturnImmediate(Scheduler &sched) {

    if (rand() % 2 == 0)
    {
        // Once every two iterations, sleep for a bit
        // to see if it messes up the scheduler.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Record start time
    Scheduler::time_point start = steady_clock::now();

    // Keep track of whether the background task was completed.
    volatile bool asyncExecuted = false;

    // Schedule the call
    sched.callAsap([&]() {

        // A negligeable amount of time should have passed
        EXPECT_GT(start + milliseconds(1), steady_clock::now());

        // Sleep 50ms in Scheduler thread
        std::__1::this_thread::sleep_for(milliseconds(50));

        // Set flag to true
        asyncExecuted = true;
    }, true);

    // Right after the call, this should still be false
    EXPECT_FALSE(asyncExecuted);

    // A negligeable amount of time should have passed
    EXPECT_GT(start + std::chrono::milliseconds(1), Scheduler::clock::now());

    // Wait for longer than the task should take to complete
    std::__1::this_thread::sleep_for(milliseconds(100));

    // It should have executed by now
    EXPECT_TRUE(asyncExecuted);
}

#endif