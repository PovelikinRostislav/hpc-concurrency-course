#include <atomic>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

volatile std::atomic<unsigned long> x(0);

class Spinlock {
  public:
  Spinlock() : flag_(ATOMIC_FLAG_INIT) {}
  void lock() { while (flag_.test_and_set()) ; }
  void unlock() { flag_.clear(); }
  private:
  std::atomic_flag flag_;
};

Spinlock S;

void BM_nolock(benchmark::State& state) {
  volatile unsigned long x = 0;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(++x);
  }
}

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(++x);
  }
}

BENCHMARK(BM_nolock) ARGS(1);
BENCHMARK(BM_lock) ARGS(1);
BENCHMARK(BM_lock) ARGS(2);
BENCHMARK(BM_lock) ARGS(4);
BENCHMARK(BM_lock) ARGS(8);
BENCHMARK(BM_lock) ARGS(16);
BENCHMARK(BM_lock) ARGS(32);
BENCHMARK(BM_lock) ARGS(64);
BENCHMARK(BM_lock) ARGS(128);

BENCHMARK_MAIN()
