#include <pthread.h>
#include <thread.h>
#include <thread>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Arg(N) \
  ->UseRealTime()

class MyThread : public Threads::Thread {
  public:
  MyThread(int* data, size_t from, size_t to) : data_(data), from_(from), to_(to) { Start(); }
  ~MyThread() { Join(); }
  private:
  virtual void Run() {
    for (size_t i = from_; i < to_; ++i) ++data_[i];
  }
  private:
  int* const data_;
  const size_t from_;
  const size_t to_;
};

void BM_thread(benchmark::State& state) {
  size_t num_threads = state.range_x();
  const size_t N = 1 << 10;
  int data[N]; for (size_t i = 0; i < N; ++i) data[i] = i;
  MyThread** threads = new MyThread*[num_threads];
  while (state.KeepRunning()) {
    for (size_t i = 0, n0 = 0, n1 = N/num_threads; i < num_threads; ++i, n0 = n1, n1 += N/num_threads) {
      threads[i] = new MyThread(data, n0, n1);
    }
    for (size_t i = 0; i < num_threads; ++i) {
      delete threads[i];
    }
  }
  delete [] threads;
}

BENCHMARK(BM_thread) ARGS(1);
BENCHMARK(BM_thread) ARGS(2);
BENCHMARK(BM_thread) ARGS(4);
BENCHMARK(BM_thread) ARGS(8);
BENCHMARK(BM_thread) ARGS(64);
BENCHMARK(BM_thread) ARGS(512);

BENCHMARK_MAIN()
