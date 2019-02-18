#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>

#include "benchmark/benchmark.h"

#define ARGS \
  ->RangeMultiplier(2)->Range(1024, 2*1024*1024) \
  ->UseRealTime()

template <class Word>
void BM_write_seq(benchmark::State& state) {
  void* memory; 
  if (::posix_memalign(&memory, 64, state.range_x()) != 0) return;
  void* const end = static_cast<char*>(memory) + state.range_x();
  Word* const p0 = static_cast<Word*>(memory);
  Word* const p1 = static_cast<Word*>(end);
  Word fill; ::memset(&fill, 0xab, sizeof(fill));
  while (state.KeepRunning()) {
    for (Word* p = p0; p < p1; ++p) {
      benchmark::DoNotOptimize(*p = fill);
    }
  }
  ::free(memory);
}

template <class Word>
void BM_read_seq(benchmark::State& state) {
  void* memory; 
  if (::posix_memalign(&memory, 64, state.range_x()) != 0) return;
  void* const end = static_cast<char*>(memory) + state.range_x();
  Word* const p0 = static_cast<Word*>(memory);
  Word* const p1 = static_cast<Word*>(end);
  Word sink; (void)sink;
  while (state.KeepRunning()) {
    for (Word* p = p0; p < p1; ++p) {
      benchmark::DoNotOptimize(sink = *p);
    }
  }
  ::free(memory);
}

//BENCHMARK_TEMPLATE1(BM_write_seq, char) ARGS;
//BENCHMARK_TEMPLATE1(BM_write_seq, short) ARGS;
//BENCHMARK_TEMPLATE1(BM_write_seq, int) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, long) ARGS;
//BENCHMARK_TEMPLATE1(BM_write_seq, __m128i) ARGS;
//BENCHMARK_TEMPLATE1(BM_write_seq, __m256i) ARGS;
//BENCHMARK_TEMPLATE1(BM_write_seq, void) ARGS;

BENCHMARK_TEMPLATE1(BM_read_seq, long) ARGS;

BENCHMARK_MAIN()
