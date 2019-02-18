#include <concurrent_queue.h>

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

concurrent_queue<large_entry_t> cq(1 << 10);

void BM_concurrent_queue(benchmark::State& state) {
  if (state.thread_index == 0) cq.add(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(large_test1(state, cq));
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
