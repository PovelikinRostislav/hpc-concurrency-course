#include <pthread.h>
#include <thread.h>
#include <thread>

#include "benchmark/benchmark.h"

//  ->Repetitions(10) 
#define ARGS \
  ->UseRealTime()

void* NoWork(void*) { return NULL; }

void BM_pthread(benchmark::State& state) {
  while (state.KeepRunning()) {
    pthread_t thread;
    pthread_create( &thread, NULL, NoWork, NULL );
    pthread_join( thread, NULL );
  }
}

class MyThread : public Threads::Thread {
  public:
  MyThread() { Start(); }
  ~MyThread() { Join(); }
  private:
  virtual void Run() {}
};

void BM_thread(benchmark::State& state) {
  while (state.KeepRunning()) {
    MyThread t;
  }
}

void NoWork1() {}

void BM_Cppthread(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::thread t(NoWork1);
    t.join();
  }
}


BENCHMARK(BM_pthread) ARGS;
BENCHMARK(BM_thread) ARGS;
BENCHMARK(BM_Cppthread) ARGS;

BENCHMARK_MAIN()
