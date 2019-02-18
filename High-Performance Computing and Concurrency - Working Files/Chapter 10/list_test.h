void BM_push_front(benchmark::State& state) {
  if (state.thread_index == 0) l.clear();
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(l.push_front(entry_t(42)));
  }
}

void BM_push_pop_front(benchmark::State& state) {
  if (state.thread_index == 0) l.clear();
  entry_t x(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(l.push_front(entry_t(42)));
    benchmark::DoNotOptimize(l.pop_front(x));
  }
}

void BM_push_pop_front1(benchmark::State& state) {
  if (state.thread_index == 0) l.clear();
  if (state.thread_index == 0) l.push_front(entry_t(0));
  if (state.thread_index == 0) l.push_front(entry_t(1));
  entry_t x(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(l.push_front(entry_t(42)));
    benchmark::DoNotOptimize(l.pop_front(x));
  }
}

void BM_empty_pop_front(benchmark::State& state) {
  if (state.thread_index == 0) l.clear();
  entry_t x(1);
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(l.pop_front(x));
  }
}

static const int Nfind = 1000;
void BM_find(benchmark::State& state) {
  if (state.thread_index == 0) {
    l.clear();
    for (int i = 0; i < Nfind; ++i) l.push_front(entry_t(i));
  }
  entry_t x(Nfind);
  while (state.KeepRunning()) {
      benchmark::DoNotOptimize(l.find(x));
  }
}

#define ALL_BENCHMARKS(N) \
BENCHMARK(BM_push_front) ARGS(N); \
BENCHMARK(BM_push_pop_front) ARGS(N); \
BENCHMARK(BM_push_pop_front1) ARGS(N); \
BENCHMARK(BM_empty_pop_front) ARGS(N); \
BENCHMARK(BM_find) ARGS(N); \
struct dummy##N {}

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
