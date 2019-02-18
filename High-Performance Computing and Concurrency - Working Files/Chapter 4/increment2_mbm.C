#include <pthread.h>
#include <thread.h>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Arg(N) \
  ->UseRealTime()

volatile unsigned long sum = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

class MyThread : public Threads::Thread {
  public:
  MyThread(size_t N) : N_(N) { Start(); }
  ~MyThread() { Join(); }
  private:
  virtual void Run() {
    for (size_t i = 0; i < N_; ++i) {
      pthread_mutex_lock(&mutex);
      ++sum;
      pthread_mutex_unlock(&mutex);
    }
  }
  private:
  const size_t N_;
};

void BM_thread(benchmark::State& state) {
  size_t num_threads = state.range_x();
  const size_t N = 1 << 20;
  MyThread** threads = new MyThread*[num_threads];
  while (state.KeepRunning()) {
    for (size_t i = 0; i < num_threads; ++i) {
      threads[i] = new MyThread(N/num_threads);
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

BENCHMARK_MAIN()
