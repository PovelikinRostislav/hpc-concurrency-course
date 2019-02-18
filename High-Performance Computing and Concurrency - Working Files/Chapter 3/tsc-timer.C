#include <tsc-timer.h>

#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include <timers.h>

// Simplified version of the real CHECK_EQ.
#define CHECK_EQ(x, y) if (!(x == y)) { std::cout << "\n[" << __FILE__ << ":" << __LINE__ << "] CHECK FAILED: " << #x << " < " << #y << " (" << #x << "=" << (x) << ", " << #y << "=" << (y) << "). "  << std::endl; abort(); }

// Compute clock cycle.
double FastTSCTimer::clock_cycle_ = 0;
double FastTSCTimer::InitClockCycle()
{
    clock_cycle_ = 1; // So the constructor does not call InitClockCycle() again.
    FastTSCTimer T;
    HighResRealTimer HT;
    T.Start(); double t1 = HT.Time();
    sleep(1);
    unsigned long t = T.Stop(); double t2 = HT.Time();
    clock_cycle_ = (t2 - t1)/(t*1e-9); // nanoseconds
    return clock_cycle_;
}

FastTSCTimer::FastTSCTimer() {
    if (clock_cycle_ == 0) InitClockCycle();
}

double AccurateTSCTimer::clock_cycle_ = 0;
double AccurateTSCTimer::InitClockCycle()
{
    clock_cycle_ = 1; // So the constructor does not call InitClockCycle() again.
    AccurateTSCTimer T;
    HighResRealTimer HT;
    T.Start(); double t1 = HT.Time();
    sleep(1);
    unsigned long t = T.Stop(); double t2 = HT.Time();
    clock_cycle_ = (t2 - t1)/(t*1e-9); // nanoseconds
    return clock_cycle_;
}

AccurateTSCTimer::AccurateTSCTimer() {
    if (clock_cycle_ == 0) InitClockCycle();
}

CPU_Limiter::CPU_Limiter()
{
    // Save CPU affinity.
    CHECK_EQ(0, ::sched_getaffinity(::getpid(), sizeof(saved_cpu_), &saved_cpu_));
    // Save interrupt mask.
    CHECK_EQ(0, ::sigprocmask(SIG_BLOCK, NULL, &saved_sigmask_));
    // Restrict the program to one CPU.
    cpu_set_t new_mask; CPU_ZERO(&new_mask); CPU_SET(0, &new_mask);
    CHECK_EQ(0, ::sched_setaffinity(::getpid(), sizeof(new_mask), &new_mask));
    // Block all signals.
    sigset_t block; ::sigfillset(&block);
    sigdelset( &block, SIGPROF ); sigdelset( &block, SIGSEGV ); sigdelset( &block, SIGBUS ); sigdelset( &block, SIGTERM );
    CHECK_EQ(0, ::sigprocmask(SIG_BLOCK, &block, NULL));
}

CPU_Limiter::~CPU_Limiter()
{
    // Restore CPU affinity.
    CHECK_EQ(0, ::sched_setaffinity(::getpid(), sizeof(saved_cpu_), &saved_cpu_));
    // Restore signal mask.
    CHECK_EQ(0, ::sigprocmask(SIG_SETMASK, &saved_sigmask_, NULL));
}
