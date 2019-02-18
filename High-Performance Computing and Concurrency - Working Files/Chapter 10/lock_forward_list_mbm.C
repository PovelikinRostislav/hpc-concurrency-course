#include <string.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <forward_list>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

using namespace std;

#include <list_test_utils.h>

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

  private:
  Spinlock s_;
  std::forward_list<entry_t> l_;
};
std_forward_list_spinlock l;

#include <list_test.h>
