#include <atomic_queue1.h>

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

struct entry_t {
  entry_t(int x = 0) : x(x) { ::memset(pad, 0, sizeof(pad)); }
  int x;
  int pad[1];
};
bool operator==(const entry_t& a, const entry_t& b) { return a.x == b.x; }
//typedef int entry_t;
typedef queue<entry_t> queue_t;

template <typename Q> bool test1(benchmark::State& state, Q& q) {
  entry_t x = 0;
  if (state.thread_index == 0) {
    if (!q.add(2)) state.SkipWithError("Queue is full");
  } else {
    q.get(x);
  }
  return x == 42;
}

class std_queue_mutex {
  public:
  bool add(const entry_t& x) {
    lock_guard<mutex> l(m_);
    q_.push(x);
    return true;
  }
  bool get(entry_t& x) {
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
  queue_t q_;
};
std_queue_mutex sqm;

void BM_std_queue_mutex(benchmark::State& state) {
  if (state.thread_index == 0) sqm.add(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(test1(state, sqm));
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
  bool add(const entry_t& x) {
    lock_guard<Spinlock> l(s_);
    q_.push(x);
    return true;
  }
  bool get(entry_t& x) {
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
  queue_t q_;
};
std_queue_spinlock sqs;

void BM_std_queue_spinlock(benchmark::State& state) {
  if (state.thread_index == 0) sqs.add(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(test1(state, sqs));
  }
}

atomic_queue1<entry_t> aq(1 << 30);

void BM_concurrent_queue(benchmark::State& state) {
  if (state.thread_index == 0) aq.add(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(test1(state, aq));
  }
}

#define ALL_BENCHMARKS(N) \
BENCHMARK(BM_std_queue_mutex) ARGS(N);          \
BENCHMARK(BM_std_queue_spinlock) ARGS(N);       \
BENCHMARK(BM_concurrent_queue) ARGS(N)

ALL_BENCHMARKS(2);

BENCHMARK_MAIN()
