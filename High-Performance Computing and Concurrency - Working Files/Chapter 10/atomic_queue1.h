#ifndef ATOMIC_QUEUE1_H_
#define ATOMIC_QUEUE1_H_

#include <atomic>
#include <memory>

// One producer, one consumer.
template <typename T> class atomic_queue1 {
  public:
  atomic_queue1(size_t capacity) : capacity_(capacity), mem_((T*)malloc(capacity*sizeof(T))), begin_(0), end_(0) {}
  ~atomic_queue1() { free(mem_); }
  bool add(const T& x) {
    size_t i = end_.load(std::memory_order_relaxed);
    if (i >= capacity_) return false;
    new (mem_ + i) T(x);
    end_.fetch_add(1, std::memory_order_release);
    return true;
  }
  bool get(T& x) {
    if (begin_ >= end_.load(std::memory_order_acquire)) return false;
    size_t i = begin_++;
    x = mem_[i];
    return true;
  }
  size_t size() {
    return end_.load(std::memory_order_relaxed) - begin_;
  }

  private:
  const size_t capacity_;
  T* const mem_;
  size_t begin_;
  std::atomic<size_t> end_;
};

#endif // ATOMIC_QUEUE1_H_ 
