// Lock-free queue with begin and end indices, testing for the entry being initialized, CAS on begin, many consumer threads
#include <string.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

using namespace std;

#include <queue_test_utils.h>

class concurrent_queue {
  enum { capacity = 1 << 20 };
  enum { mask = capacity - 1 };
  public:
  concurrent_queue() : mem_((entry_t*)malloc(capacity*sizeof(entry_t))), begin_(0), end_(0) {}
  ~concurrent_queue() { free(mem_); }
  bool add(const entry_t& x) {
    size_t i = end_.fetch_add(1, std::memory_order_relaxed);
    new (mem_ + (i & mask)) entry_t(x);
    return true;
  }
  bool get(entry_t& x) {
    size_t i = begin_.load(std::memory_order_relaxed);
    do {
      if (i >= end_.load(std::memory_order_relaxed)) return false;
      if (mem_[i & mask].x.load(std::memory_order_acquire) == 0) return false;
    } while (!begin_.compare_exchange_strong(i, i + 1, std::memory_order_relaxed));
    x = mem_[i & mask];
    return true;
  }
  size_t size() {
    return end_.load(std::memory_order_relaxed) - begin_.load(std::memory_order_relaxed);
  }

  private:
  entry_t* const mem_;
  std::atomic<size_t> begin_;
  char dummy1_[64];
  std::atomic<size_t> end_;
};
concurrent_queue cq;

void BM_concurrent_queue(benchmark::State& state) {
  if (state.thread_index == 0) cq.add(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(test1(state, cq));
  }
}

#define ALL_BENCHMARKS(N) \
BENCHMARK(BM_concurrent_queue) ARGS(N)

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
