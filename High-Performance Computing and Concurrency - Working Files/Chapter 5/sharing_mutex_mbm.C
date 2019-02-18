#include <mutex>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

const size_t N = 1 << 10;
unsigned long x;

std::mutex M;

void BM_mutex(benchmark::State& state) {
  if (state.thread_index == 0) x = 0;
  while (state.KeepRunning()) {
    for (size_t i = 0; i < N; ++i) {
      REPEAT({ std::lock_guard<std::mutex> L(M); benchmark::DoNotOptimize(++x); });
    }
  }
}

BENCHMARK(BM_mutex) ARGS(1);
BENCHMARK(BM_mutex) ARGS(2);
BENCHMARK(BM_mutex) ARGS(4);
BENCHMARK(BM_mutex) ARGS(8);

BENCHMARK_MAIN()
