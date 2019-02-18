#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>

#include "benchmark/benchmark.h"

template <class Word>
void BM_increment(benchmark::State& state) {
  Word x = 0;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(++x);
  }
}

BENCHMARK_TEMPLATE1(BM_increment, long)->Threads(1);
BENCHMARK_TEMPLATE1(BM_increment, long)->Threads(2);
BENCHMARK_TEMPLATE1(BM_increment, long)->Threads(4);
BENCHMARK_TEMPLATE1(BM_increment, long)->Threads(6);
BENCHMARK_TEMPLATE1(BM_increment, long)->Threads(8);

BENCHMARK_MAIN()
