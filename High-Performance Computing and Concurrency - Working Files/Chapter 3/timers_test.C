#include <timers.h>

#include <math.h>
#include <unistd.h>

#include <gtest/gtest.h>

// Unit tests for high-resolution timers.

TEST(Timers, RealTime) {
    HighResRealTimer T;
    EXPECT_EQ(0u, sleep(1)); // This can fail if sleep() was interrupted
    double t = T.Time();
    uint64_t ns = T.TimeNS();
    EXPECT_GE(1e-3, fabs(t - 1)) << "t=" << t; // Sleep should be accurate enough
    EXPECT_LE(1e-9, fabs(t - 1e9*ns)) << "t=" << t << " ns=" << ns;
}

TEST(Timers, CPUTimeSleep) {
    HighResCPUTimer T;
    EXPECT_EQ(0u, sleep(1)); // This can fail if sleep() was interrupted
    double t = T.Time();
    uint64_t ns = T.TimeNS();
    EXPECT_GE(1e-3, t); // Sleep should take no CPU time
    EXPECT_LE(1e-9, fabs(t - 1e9*ns)) << "t=" << t << " ns=" << ns;
}

static void DoWork() {
    volatile double x = 1;
    for (size_t i = 0; i < 1UL << 26; ++i) x *= 2; // Should take more than 0.01s
}

TEST(Timers, CPUTimeWork) {
    HighResCPUTimer T;
    DoWork();
    double t = T.Time();
    uint64_t ns = T.TimeNS();
    EXPECT_LE(1e-2, t);
    EXPECT_LE(1e-9, fabs(t - 1e9*ns)) << "t=" << t << " ns=" << ns;
}

