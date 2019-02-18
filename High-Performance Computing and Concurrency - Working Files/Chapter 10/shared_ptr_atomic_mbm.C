#include <atomic>
#include <memory>

#include "benchmark/benchmark.h"

#define REPEAT2(x) {x} {x}
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT64(x) REPEAT32(x) REPEAT32(x)
#define REPEAT(x) REPEAT64(x)

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

using namespace std;

struct A {
  int i;
  A(int i = 0) : i(i) {}
  A& operator=(const A& rhs) { i = rhs.i; return *this; }
  volatile A& operator=(const A& rhs) volatile { i = rhs.i; return *this; }
};

shared_ptr<A> p3(new A(42));

void BM_shared_ptr_atomic_deref(benchmark::State& state) {
  volatile A x;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(x = *(std::atomic_load_explicit(&p3, std::memory_order_relaxed)));
  }
}

void BM_shared_ptr_atomic_copy(benchmark::State& state) {
  while (state.KeepRunning()) {
    volatile shared_ptr<A> q(std::atomic_load_explicit(&p3, std::memory_order_relaxed));
  }
}

shared_ptr<A> q3(new A(7));

void BM_shared_ptr_atomic_assign(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::atomic_store(&q3, std::atomic_load_explicit(&p3, std::memory_order_relaxed));
  }
}

void BM_shared_ptr_atomic_assign1(benchmark::State& state) {
  while (state.KeepRunning()) {
      std::atomic_store_explicit(&q3, std::atomic_load_explicit(&p3, std::memory_order_relaxed), std::memory_order_relaxed);
  }
}

void BM_shared_ptr_atomic_xassign(benchmark::State& state) {
  if (state.thread_index == 0) p3 = shared_ptr<A>(new A(42)), q3 = shared_ptr<A>(new A(7));
  if (state.thread_index & 1) {
    while (state.KeepRunning()) {
      std::atomic_store(&q3, std::atomic_load_explicit(&p3, std::memory_order_relaxed));
    }
  } else {
    while (state.KeepRunning()) {
      std::atomic_store(&p3, std::atomic_load_explicit(&q3, std::memory_order_relaxed));
    }
  }
}

#define ALL_BENCHMARKS(N) \
BENCHMARK(BM_shared_ptr_atomic_deref) ARGS(N);      \
BENCHMARK(BM_shared_ptr_atomic_copy) ARGS(N);       \
BENCHMARK(BM_shared_ptr_atomic_assign) ARGS(N);     \
BENCHMARK(BM_shared_ptr_atomic_assign1) ARGS(N);    \
BENCHMARK(BM_shared_ptr_atomic_xassign) ARGS(N);    \
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
