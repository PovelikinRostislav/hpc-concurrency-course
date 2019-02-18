#include <atomic>

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


