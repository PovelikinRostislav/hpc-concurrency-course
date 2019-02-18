#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>

#include "benchmark/benchmark.h"

template <class Word>
void BM_increment(benchmark::State& state) {
  volatile Word x = 0; (void) x;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(++x);
  }
}

//BENCHMARK_TEMPLATE1(BM_increment, char);
//BENCHMARK_TEMPLATE1(BM_increment, short);
//BENCHMARK_TEMPLATE1(BM_increment, int);
BENCHMARK_TEMPLATE1(BM_increment, long);
//BENCHMARK_TEMPLATE1(BM_increment, __m128i);
//BENCHMARK_TEMPLATE1(BM_increment, __m256i);

BENCHMARK_MAIN()
