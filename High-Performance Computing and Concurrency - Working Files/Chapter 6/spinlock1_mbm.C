#include <time.h>
#include <atomic>
#include <mutex>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

volatile unsigned long* p = new unsigned long;
volatile unsigned long& x = *p;

class Spinlock {
  public:
  Spinlock() : flag_(ATOMIC_FLAG_INIT) {}
  void lock() {
    static const timespec ns = { 0, 1 };
    for (register int i = 0; flag_.test_and_set(); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
  }
  void unlock() { flag_.clear(); }
  private:
  std::atomic_flag flag_;
};

Spinlock S;

void BM_nolock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(++x);
  }
}

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    std::lock_guard<Spinlock> L(S);
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
