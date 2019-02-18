// Non-google-benchmark test of atomic vs spinlocked list (sanity check)
#include <atomic-forward-list.h>

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

#include <list_test_utils.h>

atomic_forward_list<entry_t> afl;

class Spinlock {
  public:
  Spinlock() : flag_(0) {}
  void lock() {
    static const timespec ns = { 0, 1 };
    for (register int i = 0; flag_.load(std::memory_order_relaxed) || flag_.exchange(1, std::memory_order_acquire); ++i) {
      if (i == 8) {
        i = 0;
        nanosleep(&ns, NULL);
      }
    }
  }
  void unlock() { flag_.store(0, std::memory_order_release); }
  private:
  std::atomic<unsigned int> flag_;
};

class std_forward_list_spinlock {
  public:
  typedef std::forward_list<entry_t>::iterator iterator;

  bool push_front(const entry_t& x) {
    std::lock_guard<Spinlock> l(s_);
    l_.push_front(x);
    return true;
  }
  bool pop_front(entry_t& x) {
    std::lock_guard<Spinlock> l(s_);
    if (l_.empty()) return false;
    x = l_.front();
    l_.pop_front();
    return true;
  }
  bool empty() {
    std::lock_guard<Spinlock> l(s_);
    return l_.empty();
  }
  void clear() {
      l_.clear();
  }
  iterator find(const entry_t& x) {
    std::lock_guard<Spinlock> l(s_);
    return std::find(l_.begin(), l_.end(), x);
  }
  iterator end() {
    std::lock_guard<Spinlock> l(s_);
    return l_.end();
  }

  private:
  Spinlock s_;
  std::forward_list<entry_t> l_;
};
std_forward_list_spinlock sfl;

Spinlock iolock;
ostream& operator<<(ostream& out, const timeval& t) {
    out << (t.tv_sec*1000000UL + t.tv_usec);
    return out;
}
long operator-(const timeval& a, const timeval& b) {
    return (a.tv_sec - b.tv_sec)*1000000UL + (a.tv_usec - b.tv_usec);
}

static const int Nfind = 1000;
template <typename L> void Prep(L* l) {
  l->clear();
  for (int i = 0; i < Nfind; ++i) l->push_front(entry_t(i));
}

volatile int sink = 0;
template <typename L> void list_find(L* l, size_t N) {
  entry_t x(Nfind);
  //timeval t0, t1;
  //gettimeofday(&t0, NULL);
  for (size_t i = 0; i < N; ++i) {
    typename L::iterator it = l->find(x);
    if (it != l->end()) sink = it->x.load();
    assert(it == l->end());
  }
  //gettimeofday(&t1, NULL);
  //std::lock_guard<Spinlock> ioguard(iolock);
  //cout << "Thread " << pthread_self() << " t=" << (t1 - t0) << endl;
}

const size_t nrep = 100000;
template <typename L> void Test(L* l, size_t NT) {
  timeval tv0, tv1;
  gettimeofday(&tv0, NULL);
  thread** t = new thread*[NT];
  for (size_t i = 0; i < NT; ++i) t[i] = new thread(list_find<L>, l, nrep);
  for (size_t i = 0; i < NT; ++i) t[i]->join();
  for (size_t i = 0; i < NT; ++i) delete t[i];
  delete [] t;
  gettimeofday(&tv1, NULL);
  std::lock_guard<Spinlock> ioguard(iolock);
  cout << NT << " threads t=" << (tv1 - tv0) << endl;
}

int main() {
  Prep(&afl);
  Prep(&sfl);
  cout << "Spinlock list:" << endl;
  Test(&sfl, 1);
  Test(&sfl, 2);
  Test(&sfl, 4);
  Test(&sfl, 8);
  Test(&sfl, 16);
  Test(&sfl, 32);
  Test(&sfl, 64);
  Test(&sfl, 80);
  Test(&sfl, 120);
  Test(&sfl, 128);
  cout << "Atomic list:" << endl;
  Test(&afl, 1);
  Test(&afl, 2);
  Test(&afl, 4);
  Test(&afl, 8);
  Test(&afl, 16);
  Test(&afl, 32);
  Test(&afl, 64);
  Test(&afl, 80);
  Test(&afl, 120);
  Test(&afl, 128);
}
