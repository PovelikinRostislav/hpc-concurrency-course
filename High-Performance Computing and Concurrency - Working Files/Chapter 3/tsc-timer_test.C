#include <tsc-timer.h>

#include <iostream>

#include <gtest/gtest.h>

using namespace std; 

static const int N = 10;

TEST(TSCTimerTest, ResolutionAccurate) {
    CPU_Limiter L;
    unsigned long t;
    AccurateTSCTimer T;
    T.Start();
    t = T.Stop();
    T.Start();
    t = T.Stop();
    EXPECT_GE(200u, t);
}

TEST(TSCTimerTest, ResolutionFast) {
    CPU_Limiter L;
    unsigned long t;
    FastTSCTimer T;
    T.Start();
    t = T.Stop();
    EXPECT_GE(200u, t);
}

TEST(TSCTimerTest, CostAccurate) {
    CPU_Limiter L;
    unsigned long t;
    AccurateTSCTimer T;
    AccurateTSCTimer T1;
    T.Start();
    t = T.Stop();
    T.Start();
    T1.Start();
    t = T.Stop();
    EXPECT_GE(800u, t);
    EXPECT_LE(200u, t);
}

TEST(TSCTimerTest, CostFast) {
    CPU_Limiter L;
    unsigned long t;
    AccurateTSCTimer T;
    FastTSCTimer T1;
    T.Start();
    t = T.Stop();
    T.Start();
    T1.Start();
    t = T.Stop();
    EXPECT_LE(100u, t);
}
