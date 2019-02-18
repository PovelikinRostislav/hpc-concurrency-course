// Non-google-benchmark test of atomic vs spinlocked increment (sanity check)
#include <string.h>
#include <sys/time.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <forward_list>
#include <mutex>
#include <thread>
#include <iostream>

using namespace std;

class aint {
  public:
  aint() : p(new std::atomic<unsigned long>) {}
  unsigned long incr() {
      return ++*p;
  }
  std::atomic<unsigned long>* p;
};

class Ptrlock {
  public:
  Ptrlock(std::atomic<unsigned long*>& p) : p_(p), p_save_(NULL) {}
  unsigned long* lock() {
    static const timespec ns = { 0, 1 };
    for (register int i = 0; !p_.load(std::memory_order_relaxed) || !(p_save_ = p_.exchange(NULL, std::memory_order_acquire)); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
    return p_save_;
  }
  void unlock() { p_.store(p_save_, std::memory_order_release); }
  private:
  std::atomic<unsigned long*>& p_;
  unsigned long* p_save_;
};

class lint {
  public:
  lint() : p(new unsigned long), L(p) {}
  unsigned long incr() { 
    unsigned long* pl = L.lock();
    unsigned long r = ++*pl;
    L.unlock();
    return r;
  }
  std::atomic<unsigned long*> p;
  Ptrlock L;
};

std::mutex iolock;
ostream& operator<<(ostream& out, const timeval& t) {
    out << (t.tv_sec*1000000UL + t.tv_usec);
    return out;
}
long operator-(const timeval& a, const timeval& b) {
    return (a.tv_sec - b.tv_sec)*1000000UL + (a.tv_usec - b.tv_usec);
}

volatile int sink = 0;
template <typename T> void incr(T* x, size_t N) {
  //timeval t0, t1;
  //gettimeofday(&t0, NULL);
  for (size_t i = 0; i < N; ++i) {
    unsigned long y = x->incr();
    if (y == 0) sink = y;
  }
  //gettimeofday(&t1, NULL);
  //std::lock_guard<std::mutex> ioguard(iolock);
  //cout << "Thread " << pthread_self() << " t=" << (t1 - t0) << endl;
}

const size_t nrep = 10000000;
template <typename T> void Test(T* x, size_t NT) {
  timeval tv0, tv1;
  gettimeofday(&tv0, NULL);
  thread** t = new thread*[NT];
  for (size_t i = 0; i < NT; ++i) t[i] = new thread(incr<T>, x, nrep);
  for (size_t i = 0; i < NT; ++i) t[i]->join();
  for (size_t i = 0; i < NT; ++i) delete t[i];
  delete [] t;
  gettimeofday(&tv1, NULL);
  std::lock_guard<std::mutex> ioguard(iolock);
  cout << NT << " threads t=" << (tv1 - tv0) << endl;
}

int main() {
  cout << "Atomic increment:" << endl;
  aint ai;
  Test(&ai, 1);
  Test(&ai, 2);
  Test(&ai, 4);
  Test(&ai, 8);
  Test(&ai, 16);
  Test(&ai, 32);
  Test(&ai, 64);
  Test(&ai, 80);
  Test(&ai, 120);
  Test(&ai, 128);
  cout << "Spinlock increment:" << endl;
  lint li;
  Test(&li, 1);
  Test(&li, 2);
  Test(&li, 4);
  Test(&li, 8);
  Test(&li, 16);
  Test(&li, 32);
  Test(&li, 64);
  Test(&li, 80);
  Test(&li, 120);
  Test(&li, 128);
}
