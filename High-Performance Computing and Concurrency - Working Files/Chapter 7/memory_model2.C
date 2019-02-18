#include <limits.h>
#include <iostream>

#include <thread.h>

using namespace std;

pthread_mutex_t M = PTHREAD_MUTEX_INITIALIZER;

volatile unsigned long x1(0);
volatile unsigned long x2(0);

class Thread1 : public Threads::Thread {
  public:
  Thread1() { Start(); }
  ~Thread1() { Join(); }
  private:
  virtual void Run() {
    for (unsigned long i = 0; i != ULONG_MAX; ++i) {
      x1 = i;
      x2 = i;
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
      unsigned long x2a = x2;
      unsigned long x1l = x1;
      unsigned long x2b = x2;
      if (x2a <= x1l && x1l <= x2b) continue; // OK
      if (x2a <= x2b && x1l == x2b + 1) continue; // OK
      cout << "x2a=" << x2a << " x1=" << x1l << " x2b=" << x2b << endl;
      _exit(0);
    }
  }
};

int main() {
  Thread1 t1;
  Thread2 t2;
}

