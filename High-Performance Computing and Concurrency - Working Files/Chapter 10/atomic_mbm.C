#include <atomic>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

std::atomic<unsigned long>* p(new std::atomic<unsigned long>);

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) *p = 0;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(p->fetch_add(1, std::memory_order_relaxed));
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
