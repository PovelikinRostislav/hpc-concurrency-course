#include <atomic>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

std::atomic<unsigned long>* p(new std::atomic<unsigned long>);

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) *p = 0;
  int s = 0;
  static const timespec ns = { 0, 1 };
  while (state.KeepRunning()) {
    unsigned long xl = p->load(std::memory_order_relaxed);
    while (!p->compare_exchange_strong(xl, xl + 1, std::memory_order_relaxed)) {
      if (++s == 8) {
        s = 0;
        nanosleep(&ns, NULL);
      }
    }
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
