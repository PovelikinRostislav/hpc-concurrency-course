#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>

#include "benchmark/benchmark.h"

#define ARGS \
  ->RangeMultiplier(2)->Range(1024, 16*1024*1024) \
  ->UseRealTime() 

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

template <class Word>
void BM_write_seq(benchmark::State& state) {
  void* memory;
  const size_t size = state.range_x();
  if (::posix_memalign(&memory, 64, size) != 0) return;
  void* const end = static_cast<char*>(memory) + size;
  Word* const p0 = static_cast<Word*>(memory);
  Word* const p1 = static_cast<Word*>(end);
  Word fill1; ::memset(&fill1, 0xab, sizeof(fill1));
  register Word fill = fill1;
  while (state.KeepRunning()) {
    for (volatile Word* p = p0; p < p1; ) {
      REPEAT(*p++ = fill;)
    }
  }
  ::free(memory);
}

template <class Word>
void BM_write_rand(benchmark::State& state) {
  void* memory;
  const size_t size = state.range_x();
  if (::posix_memalign(&memory, 64, size) != 0) return;
  Word* const p0 = static_cast<Word*>(memory);
  Word fill1; ::memset(&fill1, 0xab, sizeof(fill1));
  register Word fill = fill1;

  const size_t N = size/sizeof(Word);
  void* memory_index;
  if (::posix_memalign(&memory_index, 64, N*sizeof(int)) != 0) return;
  int* const index = static_cast<int*>(memory_index);
  for (size_t i = 0; i < N; ++i) index[i] = rand() % N;
  int* const i1 = index + N;

  while (state.KeepRunning()) {
    for (const int* ind = index; ind < i1; ) {
      REPEAT(*(p0 + *ind++) = fill;)
    }
  }
  ::free(memory);
  ::free(memory_index);
}

typedef unsigned long Word;
BENCHMARK_TEMPLATE1(BM_write_seq, Word) ARGS;
BENCHMARK_TEMPLATE1(BM_write_rand, Word) ARGS;

BENCHMARK_MAIN()
