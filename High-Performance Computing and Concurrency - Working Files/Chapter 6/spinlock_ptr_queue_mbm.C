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
std::atomic<Queue*> q(NULL);

class Queuelock {
  public:
  Queuelock(std::atomic<Queue*>& q) : q_(q), q_save_(NULL) {}
  Queue* lock() {
    static const timespec ns = { 0, 1 };
    for (register int i = 0; !(q_save_ = q_.exchange(NULL)); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
    return q_save_;
  }
  //void unlock() { q_ = q_save_; }
  void unlock() { q_.exchange(q_save_); }
  private:
  std::atomic<Queue*>& q_;
  Queue* q_save_;
};

void BM_nolock(benchmark::State& state) {
    if (state.thread_index == 0) {
        q = new Queue;
        q.load()->push(0);
    }
    unsigned long i = 0;
    volatile unsigned long j;
    Queue* ql = q.load();
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(ql->push(++i));
        benchmark::DoNotOptimize(ql->pop(j));
    }
    if (state.thread_index == 0) {
        delete q.load();
        q = NULL;
    }
}

void BM_lock(benchmark::State& state) {
    if (state.thread_index == 0) {
        q = new Queue;
        q.load()->push(0);
    }
    unsigned long i = 0;
    volatile unsigned long j;
    Queuelock L(q);
    while (state.KeepRunning()) {
        Queue* ql = L.lock();
        benchmark::DoNotOptimize(ql->push(++i));
        benchmark::DoNotOptimize(ql->pop(j));
        L.unlock();
    }
    if (state.thread_index == 0) {
        delete q.load();
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
