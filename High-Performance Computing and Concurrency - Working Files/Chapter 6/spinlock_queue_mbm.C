#include <time.h>
#include <atomic>
#include <mutex>
#include <queue>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

class Queue {
    public:
    template <typename T> bool push(T x) { q_.push(x); return true; }
    template <typename T> bool pop(T& x) { if (q_.empty()) return false; x = q_.front(); q_.pop(); return true; }
    private:
    std::queue<unsigned long> q_;
};
Queue* q = NULL;

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
    if (state.thread_index == 0) {
        q = new Queue;
        q->push(0);
    }
    unsigned long i = 0;
    volatile unsigned long j;
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(q->push(++i));
        benchmark::DoNotOptimize(q->pop(j));
    }
    if (state.thread_index == 0) {
        delete q;
        q = NULL;
    }
}

void BM_lock(benchmark::State& state) {
    if (state.thread_index == 0) {
        q = new Queue;
        q->push(0);
    }
    unsigned long i = 0;
    volatile unsigned long j;
    while (state.KeepRunning()) {
        std::lock_guard<Spinlock> L(S);
        benchmark::DoNotOptimize(q->push(++i));
        benchmark::DoNotOptimize(q->pop(j));
    }
    if (state.thread_index == 0) {
        delete q;
        q = NULL;
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
