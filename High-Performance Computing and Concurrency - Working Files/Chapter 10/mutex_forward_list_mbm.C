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

class std_forward_list_mutex {
  public:
  typedef std::forward_list<entry_t>::iterator iterator;

  bool push_front(const entry_t& x) {
    std::lock_guard<std::mutex> l(m_);
    l_.push_front(x);
    return true;
  }
  bool pop_front(entry_t& x) {
    std::lock_guard<std::mutex> l(m_);
    if (l_.empty()) return false;
    x = l_.front();
    l_.pop_front();
    return true;
  }
  bool empty() {
    std::lock_guard<std::mutex> l(m_);
    return l_.empty();
  }
  void clear() {
      l_.clear();
  }
  iterator find(const entry_t& x) {
    std::lock_guard<std::mutex> l(m_);
    return std::find(l_.begin(), l_.end(), x);
  }

  private:
  std::mutex m_;
  std::forward_list<entry_t> l_;
};
std_forward_list_mutex l;

#include <list_test.h>
