#include <limits.h>
#include <iostream>
#include <atomic>

#include <thread.h>

using namespace std;

pthread_mutex_t M = PTHREAD_MUTEX_INITIALIZER;

atomic<volatile unsigned long> x1(0);
atomic<volatile unsigned long> x2(0);

#define ORDER memory_order_relaxed
//#define ORDER memory_order_seq_cst

class Thread1 : public Threads::Thread {
  public:
  Thread1() { Start(); }
  ~Thread1() { Join(); }
  private:
  virtual void Run() {
    for (unsigned long i = 0; i != ULONG_MAX; ++i) {
      x1.store(i, ORDER);
      unsigned long x = x1.load(ORDER);
      x2.store(x, ORDER);
    }
  }
};

class Thread2 : public Threads::Thread {
  public:
  Thread2() { Start(); }
  ~Thread2() { Join(); }
  private:
  virtual void Run() {
    while (true) {
      unsigned long x = x1.load(ORDER);
      x2.store(x, ORDER);
    }
  }
};

class Thread3 : public Threads::Thread {
  public:
  Thread3() { Start(); }
  ~Thread3() { Join(); }
  private:
  virtual void Run() {
    while (true) {
      unsigned long x1a = x1.load(ORDER);
      unsigned long x = x2.load(ORDER);
      unsigned long x1b = x1.load(ORDER);
      if (x1a <= x && x <= x1b) continue; // OK
      if (x1a <= x1b && x == x1a - 1) continue; // OK
      cout << "x1a=" << x1a << " x1=" << x << " x1b=" << x1b << endl;
      _exit(0);
    }
  }
};

int main() {
  Thread1 t1;
  //Thread2 t2;
  Thread3 t3;
}

