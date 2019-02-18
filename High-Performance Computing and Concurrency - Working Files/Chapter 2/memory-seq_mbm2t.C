#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>

#include "benchmark/benchmark.h"

#define ARGS \
  ->Arg(1024    ) \
  ->Arg(2048    ) \
  ->Arg(4096    ) \
  ->Arg(6144    ) \
  ->Arg(8192    ) \
  ->Arg(12288   ) \
  ->Arg(16384   ) \
  ->Arg(24576   ) \
  ->Arg(32768   ) \
  ->Arg(49152   ) \
  ->Arg(65536   ) \
  ->Arg(98304   ) \
  ->Arg(131072  ) \
  ->Arg(196608  ) \
  ->Arg(262144  ) \
  ->Arg(393216  ) \
  ->Arg(524288  ) \
  ->Arg(786432  ) \
  ->Arg(1048576 ) \
  ->Arg(1572864 ) \
  ->Arg(2097152 ) \
  ->Arg(3145728 ) \
  ->Arg(4194304 ) \
  ->Arg(6291456 ) \
  ->Arg(8388608 ) \
  ->Arg(12582912) \
  ->Arg(16777216) \
  ->Arg(25165824) \
  ->Arg(33554432) \
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

template <>
void BM_write_seq<void>(benchmark::State& state) {
  void* memory; 
  const size_t size = state.range_x();
  if (::posix_memalign(&memory, 64, size) != 0) return;
  while (state.KeepRunning()) {
    ::memset(memory, 0xab, size);
  }
  ::free(memory);
}

template <class Word>
void BM_read_seq(benchmark::State& state) {
  void* memory; 
  const size_t size = state.range_x();
  if (::posix_memalign(&memory, 64, size) != 0) return;
  ::memset(memory, 0xab, size);
  void* const end = static_cast<char*>(memory) + size;
  Word* const p0 = static_cast<Word*>(memory);
  Word* const p1 = static_cast<Word*>(end);
  register Word sink; (void)sink;
  while (state.KeepRunning()) {
    for (const volatile Word* p = p0; p < p1; ++p) {
      REPEAT(sink = *p++;)
    }
  }
  ::free(memory);
}

template <>
void BM_read_seq<void>(benchmark::State& state) {
  void* memory; 
  const size_t size = state.range_x();
  if (::posix_memalign(&memory, 64, size) != 0) return;
  ::memset(memory, 0xab, size);
  while (state.KeepRunning()) {
    if (::memchr(memory, 0xff, size) != NULL) exit(0);
  }
  ::free(memory);
}

BENCHMARK_TEMPLATE1(BM_write_seq, char) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, short) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, int) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, long) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, __m128i) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, __m256i) ARGS;
BENCHMARK_TEMPLATE1(BM_write_seq, void) ARGS;

//BENCHMARK_TEMPLATE1(BM_read_seq, __m128i) ARGS;
//BENCHMARK_TEMPLATE1(BM_read_seq, void) ARGS;

BENCHMARK_MAIN()
