#include <pthread.h>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

volatile unsigned long* p = new unsigned long;
volatile unsigned long& x = *p;

pthread_mutex_t M = PTHREAD_MUTEX_INITIALIZER;

void BM_nolock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(++x);
  }
}

void BM_lock(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    pthread_mutex_lock(&M);
    benchmark::DoNotOptimize(++x);
    pthread_mutex_unlock(&M);
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
