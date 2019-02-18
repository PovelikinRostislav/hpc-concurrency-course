#include <string.h>
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

A* p2(new A(42));
shared_ptr<A> p3(new A(42));

void BM_ptr_deref(benchmark::State& state) {
  volatile A x;
  while (state.KeepRunning()) {
    REPEAT(benchmark::DoNotOptimize(x = *p2););
  }
}

void BM_shared_ptr_deref(benchmark::State& state) {
  volatile A x;
  while (state.KeepRunning()) {
    REPEAT(benchmark::DoNotOptimize(x = *p3););
  }
}

void BM_ptr_copy(benchmark::State& state) {
  while (state.KeepRunning()) {
    volatile A* q(p2); (void)(q);
  }
}

void BM_shared_ptr_copy(benchmark::State& state) {
  while (state.KeepRunning()) {
    volatile shared_ptr<A> q(p3);
  }
}

volatile A* q2(new A(7));
shared_ptr<A> q3(new A(7));

void BM_ptr_assign(benchmark::State& state) {
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(q2 = p2);
  }
}

void BM_shared_ptr_assign(benchmark::State& state) {
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(q3 = p3);
  }
}

#define ALL_BENCHMARKS(N) \
BENCHMARK(BM_ptr_deref) ARGS(N);                    \
BENCHMARK(BM_shared_ptr_deref) ARGS(N);             \
BENCHMARK(BM_ptr_copy) ARGS(N);                     \
BENCHMARK(BM_shared_ptr_copy) ARGS(N);              \
BENCHMARK(BM_ptr_assign) ARGS(N);                   \
BENCHMARK(BM_shared_ptr_assign) ARGS(N);            \
struct dummy##N {}

ALL_BENCHMARKS(1);
ALL_BENCHMARKS(2);
ALL_BENCHMARKS(4);
ALL_BENCHMARKS(16);
ALL_BENCHMARKS(32);
ALL_BENCHMARKS(64);
ALL_BENCHMARKS(80);
ALL_BENCHMARKS(120);
ALL_BENCHMARKS(128);

BENCHMARK_MAIN()
