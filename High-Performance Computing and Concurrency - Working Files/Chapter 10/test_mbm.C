// Demo of how Google benchmark counts scaling:
// Perfect scaling means reported time decreases linearly with the number of threads.
// No scaling means reported time is constant.
#include <string.h>
#include <errno.h>
#include <time.h>
#include <atomic>
#include <mutex>
#include <iostream>

#include "benchmark/benchmark.h"

using namespace std;

#define REPEAT2(x) {x} {x}
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT64(x) REPEAT32(x) REPEAT32(x)
#define REPEAT(x) REPEAT64(x)

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

// This should never be seen in user space but with some buggy kernels it can be.
#ifndef ERESTART_RESTARTBLOCK
#define ERESTART_RESTARTBLOCK 516
#endif // ERESTART_RESTARTBLOCK 
#ifndef ERESTARTNOHAND
#define ERESTARTNOHAND 514
#endif // ERESTARTNOHAND 

inline void Nanosleep(timespec req) {
    struct timespec rem;
    while (nanosleep(&req, &rem) != 0) {
        if (errno == EINTR || errno == ERESTART_RESTARTBLOCK) { // ERESTART_RESTARTBLOCK escaping into userspace is a kernel bug but remaining timeout should be correct
            req = rem;
            continue;
        }
        if (errno == ERESTARTNOHAND) continue; // ERESTARTNOHAND escaping into userspace is a kernel bug, timeout should be reset
        return;
    }
}

void BM_perfect_scaling1(benchmark::State& state) {
  unsigned long x = 0;
  while (state.KeepRunning()) {
    REPEAT(REPEAT(benchmark::DoNotOptimize(++x);););
  }
}

void BM_perfect_scaling2(benchmark::State& state) {
  static const timespec t = { 0, 1000000 };
  while (state.KeepRunning()) {
    Nanosleep(t);
  }
}

std::mutex M;
void BM_no_scaling1(benchmark::State& state) {
  unsigned long x = 0;
  while (state.KeepRunning()) {
    std::lock_guard<std::mutex> L(M);
    REPEAT(REPEAT(benchmark::DoNotOptimize(++x);););
  }
}

void BM_no_scaling2(benchmark::State& state) {
  const timespec t = { 0, 1000000*state.threads };
  while (state.KeepRunning()) {
    Nanosleep(t);
  }
}

#define ALL_BENCHMARKS(N) \
BENCHMARK(BM_perfect_scaling1) ARGS(N); \
BENCHMARK(BM_perfect_scaling2) ARGS(N); \
BENCHMARK(BM_no_scaling1) ARGS(N); \
BENCHMARK(BM_no_scaling2) ARGS(N); \
struct dummy##N {}

ALL_BENCHMARKS(1);
ALL_BENCHMARKS(2);
ALL_BENCHMARKS(4);
ALL_BENCHMARKS(8);
ALL_BENCHMARKS(16);
ALL_BENCHMARKS(32);
ALL_BENCHMARKS(64);
ALL_BENCHMARKS(80);
ALL_BENCHMARKS(120);
ALL_BENCHMARKS(128);

BENCHMARK_MAIN()
