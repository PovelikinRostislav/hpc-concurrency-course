#ifndef QUEUE_TEST_UTILS_H_
#define QUEUE_TEST_UTILS_H_

struct entry_t {
  entry_t(int x = 0) : x(x) { ::memset(pad, 0, sizeof(pad)); }
  std::atomic<int> x;
  int pad[1];
  entry_t(const entry_t& rhs) {
      ::memcpy(pad, rhs.pad, sizeof(pad));
      x.store(rhs.x.load(std::memory_order_relaxed), std::memory_order_release);
  }
  entry_t& operator=(const entry_t& rhs) {
      if (this == &rhs) return *this;
      ::memcpy(pad, rhs.pad, sizeof(pad));
      x.store(rhs.x.load(std::memory_order_relaxed), std::memory_order_release);
      return *this;
  }
};
bool operator==(const entry_t& a, const entry_t& b) { return a.x == b.x; }

typedef queue<entry_t> queue_t;

struct large_entry_t {
  large_entry_t(int x = 0) : x(x) { ::memset(pad, 0, sizeof(pad)); }
  std::atomic<int> x;
  int pad[1023];
  large_entry_t(const large_entry_t& rhs) {
      ::memcpy(pad, rhs.pad, sizeof(pad));
      x.store(rhs.x.load(std::memory_order_relaxed), std::memory_order_release);
  }
  large_entry_t& operator=(const large_entry_t& rhs) {
      if (this == &rhs) return *this;
      ::memcpy(pad, rhs.pad, sizeof(pad));
      x.store(rhs.x.load(std::memory_order_relaxed), std::memory_order_release);
      return *this;
  }
};
bool operator==(const large_entry_t& a, const large_entry_t& b) { return a.x == b.x; }

typedef queue<large_entry_t> large_queue_t;

template <typename Q> bool test1(benchmark::State& state, Q& q) {
  entry_t x = 0;
  if (q.size() > 1000) {
    q.get(x);
  } else {
    if (!q.add(2)) state.SkipWithError("Queue is full");
  }
  return x == 42;
}

template <typename Q> bool large_test1(benchmark::State& state, Q& q) {
  large_entry_t x = 0;
  if (q.size() > 1000) {
    q.get(x);
  } else {
    if (!q.add(2)) state.SkipWithError("Queue is full");
  }
  return x == 42;
}

#endif // QUEUE_TEST_UTILS_H_
