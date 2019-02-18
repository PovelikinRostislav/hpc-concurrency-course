#include <time.h>
#include <atomic>
#include <mutex>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

std::atomic<unsigned long*> p(new unsigned long);

class Ptrlock {
  public:
  Ptrlock(std::atomic<unsigned long*>& p) : p_(p), p_save_(NULL) {}
  unsigned long* lock() {
    static const timespec ns = { 0, 1 };
    for (register int i = 0; !(p_save_ = p_.exchange(NULL, std::memory_order_acquire)); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
    return p_save_;
  }
  void unlock() { p_.store(p_save_, std::memory_order_release); }
  private:
  std::atomic<unsigned long*>& p_;
  unsigned long* p_save_;
};

void BM_nolock(benchmark::State& state) {
  if (state.thread_index == 0) *p.load() = 0;
  unsigned long* pl = p.load();
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(++*pl);
  }
}

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) *p.load() = 0;
  Ptrlock L(p);
  while (state.KeepRunning()) {
    unsigned long* pl = L.lock();
    benchmark::DoNotOptimize(++*pl);
    L.unlock();
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
