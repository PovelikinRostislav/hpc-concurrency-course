#include <string.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

struct entry_t {
  entry_t(int x = 0) : x(x) { ::memset(pad, 0, sizeof(pad)); }
  int x;
  int pad[1];
};
bool operator==(const entry_t& a, const entry_t& b) { return a.x == b.x; }
//typedef int entry_t;
typedef queue<entry_t> queue_t;

class std_queue_mutex {
  public:
  bool add(const entry_t& x) {
    lock_guard<mutex> l(m_);
    q_.push(x);
    return true;
  }
  bool get(entry_t& x) {
    lock_guard<mutex> l(m_);
    if (q_.empty()) return false;
    x = q_.front();
    q_.pop();
    return true;
  }
  size_t size() {
    lock_guard<mutex> l(m_);
    return q_.size();
  }

  private:
  mutex m_;
  queue_t q_;
};

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

class std_queue_spinlock {
  public:
  bool add(const entry_t& x) {
    lock_guard<Spinlock> l(s_);
    q_.push(x);
    return true;
  }
  bool get(entry_t& x) {
    lock_guard<Spinlock> l(s_);
    if (q_.empty()) return false;
    x = q_.front();
    q_.pop();
    return true;
  }
  size_t size() {
    lock_guard<Spinlock> l(s_);
    return q_.size();
  }

  private:
  Spinlock s_;
  queue_t q_;
};

class concurrent_queue {
  enum { capacity = 1 << 30 };
  public:
  concurrent_queue() : mem_((entry_t*)malloc(capacity*sizeof(entry_t))), begin_(0), end_(0) {}
  ~concurrent_queue() { free(mem_); }
  bool add(const entry_t& x) {
    size_t i = end_.fetch_add(1, std::memory_order_relaxed);
    if (i >= capacity) return false;
    new (mem_ + i) entry_t(x);
    return true;
  }
  bool get(entry_t& x) {
    size_t i = begin_.fetch_add(1, std::memory_order_relaxed);
    if (i >= end_.load(std::memory_order_relaxed)) return false;
    x = mem_[i];
    return true;
  }
  size_t size() {
    return end_.load(std::memory_order_relaxed) - begin_.fetch_add(1, std::memory_order_relaxed);
  }

  private:
  entry_t* const mem_;
  std::atomic<size_t> begin_;
  std::atomic<size_t> end_;
};

template <typename Q> void test1(Q* q, size_t N) {
  entry_t x = 0;
  for (size_t i = 0; i < N; ++i) {
    if (q->size() > 1000) {
      q->get(x);
    } else {
      if (!q->add(i)) abort();
    }
  }
}

template <typename Q> void Test1() {
  const size_t N = 10000;
  {
    Q q;
    {
      thread t1(test1<Q>, &q, N);
      thread t2(test1<Q>, &q, N);
      t1.join();
      t2.join();
    }
  }
}

int main() {
  Test1<std_queue_mutex>();
  Test1<std_queue_spinlock>();
  Test1<concurrent_queue>();
}
