#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

template <class Word>
void BM_increment(benchmark::State& state) {
  volatile Word x = 0; (void) x;
  while (state.KeepRunning()) {
    REPEAT(++x;)
  }
}

template <class Word>
void BM_write(benchmark::State& state) {
  volatile Word x = 0; (void) x;
  while (state.KeepRunning()) {
    REPEAT(x = 0;)
  }
}

template <class Word>
void BM_read_write(benchmark::State& state) {
  volatile Word x = 0; (void) x;
  volatile Word y = 0; (void) y;
  while (state.KeepRunning()) {
    REPEAT(x = 0; y = x;)
  }
}

BENCHMARK_TEMPLATE1(BM_increment, long);
BENCHMARK_TEMPLATE1(BM_write, long);
BENCHMARK_TEMPLATE1(BM_read_write, long);

BENCHMARK_MAIN()
