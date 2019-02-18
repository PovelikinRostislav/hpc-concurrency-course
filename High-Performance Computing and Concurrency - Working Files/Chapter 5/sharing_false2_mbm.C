#include <atomic>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

#define ARGS(N, M) \
  ->Threads(N) \
  ->Arg(M) \
  ->UseRealTime()

const size_t N = 1 << 10;
std::atomic<unsigned long> x[4096];

void BM_false(benchmark::State& state) {
  if (state.thread_index == 0) for (size_t i = 0; i < sizeof(x)/sizeof(x[0]); ++i) x[i].store(0);
  const size_t pos = state.range_x()*state.thread_index;
  while (state.KeepRunning()) {
    for (size_t i = 0; i < N; ++i) {
      REPEAT(benchmark::DoNotOptimize(++x[pos]););
    }
  }
}

BENCHMARK(BM_false) ARGS(2, 0);
BENCHMARK(BM_false) ARGS(2, 1);
BENCHMARK(BM_false) ARGS(2, 2);
BENCHMARK(BM_false) ARGS(2, 3);
BENCHMARK(BM_false) ARGS(2, 4);
BENCHMARK(BM_false) ARGS(2, 5);
BENCHMARK(BM_false) ARGS(2, 6);
BENCHMARK(BM_false) ARGS(2, 7);
BENCHMARK(BM_false) ARGS(2, 8);
BENCHMARK(BM_false) ARGS(2, 9);
BENCHMARK(BM_false) ARGS(2, 10);
BENCHMARK(BM_false) ARGS(2, 11);
BENCHMARK(BM_false) ARGS(2, 12);
BENCHMARK(BM_false) ARGS(2, 13);
BENCHMARK(BM_false) ARGS(2, 14);
BENCHMARK(BM_false) ARGS(2, 15);
BENCHMARK(BM_false) ARGS(2, 16);
BENCHMARK(BM_false) ARGS(2, 17);
BENCHMARK(BM_false) ARGS(2, 18);
BENCHMARK(BM_false) ARGS(2, 19);
BENCHMARK(BM_false) ARGS(2, 20);

BENCHMARK_MAIN()
