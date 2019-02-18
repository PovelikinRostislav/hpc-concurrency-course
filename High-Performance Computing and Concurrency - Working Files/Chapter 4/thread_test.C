#include <thread.h>

#include <pthread.h>

#include <common.h>

#include <gtest/gtest.h>

using namespace Threads;

class Thread1 : public Thread {
    public:
    Thread1(const Thread::Options& options = Thread::Options())
        : Thread(options), ran_(false), thread_tid_(0), caller_options_(NULL) {}

    protected:
    virtual void Run() {
        CHECK_EQ(tid(), pthread_self());
        ran_ = true;
        thread_tid_ = tid();
        if (caller_options_) *caller_options_ = options();
    }

    public:
    bool ran_;
    pthread_t thread_tid_;
    Thread::Options* caller_options_;
};

TEST(Threads, Join) {
    Thread1 t;
    EXPECT_FALSE(t.ran_);
    t.Start();
    t.Join();
    EXPECT_TRUE(t.ran_);
    EXPECT_NE(pthread_self(), t.thread_tid_);
    EXPECT_NE(0u, t.thread_tid_);
}

TEST(Threads, Detached) {
    Thread1 t(Thread::Options().set_joinable(false));
    EXPECT_FALSE(t.ran_);
    t.Start();
    for (int i = 0; i < 1000; ++i) {  // Wait a bit if needed, but not forever
        SleepForSeconds(1e-3);
        if (t.ran_) break;
    }
    EXPECT_TRUE(t.ran_);  // Fails if we timed out above
    EXPECT_NE(pthread_self(), t.thread_tid_);
    EXPECT_NE(0u, t.thread_tid_);
}

TEST(Threads, Options) {
    Thread::Options o;
    Thread1 t(Thread::Options().set_stack_size(16384));
    t.caller_options_ = &o;
    t.Start();
    t.Join();
    EXPECT_TRUE(o.joinable());
    EXPECT_EQ(16384u, o.stack_size());
}

TEST(Threads, JoinNotStarted) {
    Thread1 t;
    EXPECT_DEATH(
        {
            t.Join();
        }, "Attempting to join a non-existing thread");
}

TEST(Threads, JoinNotJoinable) {
    Thread1 t(Thread::Options().set_joinable(false));
    t.Start();
    EXPECT_DEATH(
        {
            t.Join();
        }, "Attempting to join a detached thread");
}

class Thread2 : public Thread {
    public:
    Thread2(const Thread::Options& options = Thread::Options())
        : Thread(options), ran_(false) {}

    protected:
    virtual void Run() {
        CHECK_EQ(tid(), pthread_self());
        ran_ = true;
        // Block forever.
        while (true) {
            SleepForSeconds(0.1);
        }
    }

    public:
    bool ran_;
};

TEST(Threads, Cancel) {
    Thread2 t;
    EXPECT_FALSE(t.ran_);
    t.Start();
    t.Cancel();
    t.Join();
    EXPECT_TRUE(t.ran_);
}

