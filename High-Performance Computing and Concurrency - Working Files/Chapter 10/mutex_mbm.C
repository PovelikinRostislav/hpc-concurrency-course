#include <time.h>
#include <atomic>
#include <mutex>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

std::atomic<unsigned long*> p(new unsigned long);
volatile unsigned long& x = *p;

std::mutex M;

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    std::lock_guard<std::mutex> L(M);
    benchmark::DoNotOptimize(++x);
  }
}

BENCHMARK(BM_lock) ARGS(1);
BENCHMARK(BM_lock) ARGS(2);
BENCHMARK(BM_lock) ARGS(4);
BENCHMARK(BM_lock) ARGS(8);
BENCHMARK(BM_lock) ARGS(16);
BENCHMARK(BM_lock) ARGS(32);
BENCHMARK(BM_lock) ARGS(64);
BENCHMARK(BM_lock) ARGS(80);
BENCHMARK(BM_lock) ARGS(120);
BENCHMARK(BM_lock) ARGS(128);

BENCHMARK_MAIN()
