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

class std_queue_mutex {
  public:
  bool add(const large_entry_t& x) {
    lock_guard<mutex> l(m_);
    q_.push(x);
    return true;
  }
  bool get(large_entry_t& x) {
    lock_guard<mutex> l(m_);
    if (q_.empty()) return false;
    x = q_.front();
    q_.pop();
    return true;
  }
  size_t size() {
    lock_guard<mutex> l(m_);
    return q_.size();
  }

  private:
  mutex m_;
  large_queue_t q_;
};
std_queue_mutex sqm;

void BM_std_queue_mutex(benchmark::State& state) {
  if (state.thread_index == 0) sqm.add(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(large_test1(state, sqm));
  }
}

class Spinlock {
  public:
  Spinlock() : flag_(0) {}
  void lock() {
    static const timespec ns = { 0, 1 };
    for (register int i = 0; flag_.load(std::memory_order_relaxed) || flag_.exchange(1, std::memory_order_acquire); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
  }
  void unlock() { flag_.store(0, std::memory_order_release); }
  private:
  std::atomic<unsigned int> flag_;
};

class std_queue_spinlock {
  public:
  bool add(const large_entry_t& x) {
    lock_guard<Spinlock> l(s_);
    q_.push(x);
    return true;
  }
  bool get(large_entry_t& x) {
    lock_guard<Spinlock> l(s_);
    if (q_.empty()) return false;
    x = q_.front();
    q_.pop();
    return true;
  }
  size_t size() {
    lock_guard<Spinlock> l(s_);
    return q_.size();
  }

  private:
  Spinlock s_;
  large_queue_t q_;
};
std_queue_spinlock sqs;

void BM_std_queue_spinlock(benchmark::State& state) {
  if (state.thread_index == 0) sqs.add(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(large_test1(state, sqs));
  }
}

#define ALL_BENCHMARKS(N) \
BENCHMARK(BM_std_queue_mutex) ARGS(N);          \
BENCHMARK(BM_std_queue_spinlock) ARGS(N);       \
struct dummy##N {}

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
