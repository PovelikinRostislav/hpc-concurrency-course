#include <atomic>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

volatile std::atomic<unsigned long> x(0);

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    unsigned long xl = x.load(std::memory_order_relaxed);
    while (!x.compare_exchange_strong(xl, xl + 1, std::memory_order_relaxed)) {}
  }
}

BENCHMARK(BM_lock) ARGS(1);
BENCHMARK(BM_lock) ARGS(2);
BENCHMARK(BM_lock) ARGS(4);
BENCHMARK(BM_lock) ARGS(8);
BENCHMARK(BM_lock) ARGS(16);
BENCHMARK(BM_lock) ARGS(32);
BENCHMARK(BM_lock) ARGS(64);
BENCHMARK(BM_lock) ARGS(128);

BENCHMARK_MAIN()
