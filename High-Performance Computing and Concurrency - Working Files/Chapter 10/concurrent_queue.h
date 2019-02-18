#ifndef CONCURRENT_QUEUE_H_
#define CONCURRENT_QUEUE_H_

#include <stdlib.h>
#include <time.h>
#include <atomic>

// Circular queue of a fixed size.
// This class is not thread-safe, it is supposed to be manipulated by one
// thread at a time.
template <typename T> class subqueue {
    public:
    explicit subqueue(size_t capacity) : capacity_(capacity), begin_(0), size_(0) {}
    size_t capacity() const { return capacity_; }
    size_t size() const { return size_; }
    bool add(const T& x) {
        if (size_ == capacity_) return false;
        size_t end = begin_ + size_++;
        if (end >= capacity_) end -= capacity_;
        data_[end] = x;
        return true;
    }
    bool get(T& x) {
        if (size_ == 0) return false;
        --size_;
        size_t pos = begin_;
        if (++begin_ == capacity_) begin_ -= capacity_;
        x = data_[pos];
        return true;
    }
    static size_t memsize(size_t capacity) {
        return sizeof(subqueue) + capacity*sizeof(T);
    }
    static subqueue* construct(size_t capacity) {
        return new(::malloc(subqueue::memsize(capacity))) subqueue(capacity);
    }
    static void destroy(subqueue* queue) {
        queue->~subqueue();
        ::free(queue);
    }

    // Test use only!
    size_t begin() const { return begin_; }

    private:
    const size_t capacity_;
    size_t begin_;
    size_t size_;
    T data_[1]; // Actually [capacity_]
};

// Collection of several subqueues, for optimizing of concurrent access.
// Each queue pointer is on a separate cache line.
template <typename Q> struct subqueue_ptr {
  subqueue_ptr() : queue() {}
  std::atomic<Q*> queue;
  char padding[64 - sizeof(queue)]; // Padding to cache line
};

template <typename T> class concurrent_queue {
  typedef subqueue<T> subqueue_t;
  typedef subqueue_ptr<subqueue_t> subqueue_ptr_t;
  public:
    explicit concurrent_queue(size_t capacity) : count_(), enqueue_slot_(), dequeue_slot_() {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        queues_[i].queue.store(subqueue_t::construct(capacity), std::memory_order_relaxed);
      }
    }
    ~concurrent_queue() {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        subqueue_t* queue = queues_[i].queue.exchange(nullptr, std::memory_order_relaxed);
        subqueue_t::destroy(queue);
      }
    }

    // How many entries are in the queue?
    size_t size() const { return count_.load(std::memory_order_acquire); }

    // Is the queue empty?
    bool empty() const { return size() == 0; }

    // Get an entry from the queue.
    // This method blocks until either the queue is empty (size() == 0) or an
    // entry is returned from one of the subqueues.
    bool get(T& entry) {
      if (count_.load(std::memory_order_acquire) == 0) return false;
      // Take ownership of a subqueue. The subqueue pointer is reset to NULL
      // while the calling thread owns the subqueue. When done, relinquish
      // the ownership by restoring the pointer.  The subqueue we got may be
      // empty, but this does not mean that we have no entries: we must check
      // other queues. We can exit the loop when we got a entry or the entry
      // count shows that we have no entries.
      // Note that decrementing the count is not atomic with dequeueing the
      // entries, so we might spin on empty queues for a little while until the
      // count catches up.
      subqueue_t* queue = NULL;
      bool success = false;
      //for (size_t dequeue_slot = 0, i = 0; ;)
      for (size_t i = 0; ;)
      {
        //i = ++dequeue_slot & (QUEUE_COUNT - 1);
        i = dequeue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
        queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);  // Take ownership of the subqueue
        if (queue) {
          success = queue->get(entry);                                          // Dequeue entry while we own the queue
          queues_[i].queue.store(queue, std::memory_order_release);             // Relinquish ownership
          if (success) break;                                                   // We have a entry
          if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;          // No entry, and no more left
        } else {                                                                // queue is NULL, nothing to relinquish
          if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;          // We failed to get a queue but there are no more entries left
        }
        if (success) break;
        if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;
        static const struct timespec ns = { 0, 1 };
        nanosleep(&ns, NULL);
      };
      // If we have a entry, decrement the queued entry count.
      count_.fetch_add(-1);
EMPTY:
      return success;
    }

    // Add a entry to the queue.
    // This method blocks until either the queue is full or a entry is added to
    // one of the subqueues. Note that the "queue is full" condition cannot be
    // checked atomically for all subqueues, so it's approximate, we try
    // several subqueues, if they are all full we give up.
    bool add(const T& entry) {
      // Preemptively increment the entry count: get() will spin on subqueues
      // as long as it thinks there is a entry to dequeue, but it will exit as
      // soon as the count is zero, so we want to avoid the situation when we
      // added a entry to a subqueue, have not incremented the count yet, but
      // get() exited with no entry. If this were to happen, the pool could be
      // deleted with entries still in the queue.
      count_.fetch_add(1);
      // Take ownership of a subqueue. The subqueue pointer is reset to NULL
      // while the calling thread owns the subqueue. When done, relinquish
      // the ownership by restoring the pointer.  The subqueue we got may be
      // full, in which case we try another subqueue, but don't loop forever
      // if all subqueues keep coming up full.
      subqueue_t* queue = NULL;
      bool success = false;
      int full_count = 0;                                             // How many subqueues we tried and found full
      //for (size_t enqueue_slot = 0, i = 0; ;)
      for (size_t i = 0; ;)
      {
        //i = ++enqueue_slot & (QUEUE_COUNT - 1);
        i = enqueue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
        queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);    // Take ownership of the subqueue
        if (queue) {
          success = queue->add(entry);                                            // Enqueue entry while we own the queue
          queues_[i].queue.store(queue, std::memory_order_release);               // Relinquish ownership
          if (success) return success;                                            // We added the entry
          if (++full_count == QUEUE_COUNT) break;                                 // We tried hard enough, probably queue is full
        }
        static const struct timespec ns = { 0, 1 };
        nanosleep(&ns, NULL);
      };
      // If we added the entry, the count is already incremented. Otherwise,
      // we must decrement the count now.
      count_.fetch_add(-1);
      return success;
    }

  private:
    enum { QUEUE_COUNT = 16 };
    subqueue_ptr_t queues_[QUEUE_COUNT];
    std::atomic<int> count_;
    std::atomic<size_t> enqueue_slot_;
    std::atomic<size_t> dequeue_slot_;
};

#include <queue>
template <typename T> class concurrent_std_queue {
  typedef std::queue<T> subqueue_t;
  struct subqueue_ptr_t {
    subqueue_ptr_t() : queue() {}
    std::atomic<subqueue_t*> queue;
    char padding[64 - sizeof(queue)]; // Padding to cache line
  };

  public:
    explicit concurrent_std_queue() : count_(), enqueue_slot_(), dequeue_slot_() {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        queues_[i].queue.store(new subqueue_t, std::memory_order_relaxed);
      }
    }
    ~concurrent_std_queue() {
      for (int i = 0; i < QUEUE_COUNT; ++i) {
        subqueue_t* queue = queues_[i].queue.exchange(nullptr, std::memory_order_relaxed);
        delete queue;
      }
    }

    // How many entries are in the queue?
    size_t size() const { return count_.load(std::memory_order_acquire); }

    // Is the queue empty?
    bool empty() const { return size() == 0; }

    // Get an entry from the queue.
    // This method blocks until either the queue is empty (size() == 0) or an
    // entry is returned from one of the subqueues.
    bool get(T& entry) {
      if (count_.load(std::memory_order_acquire) == 0) return false;
      // Take ownership of a subqueue. The subqueue pointer is reset to NULL
      // while the calling thread owns the subqueue. When done, relinquish
      // the ownership by restoring the pointer.  The subqueue we got may be
      // empty, but this does not mean that we have no entries: we must check
      // other queues. We can exit the loop when we got a entry or the entry
      // count shows that we have no entries.
      // Note that decrementing the count is not atomic with dequeueing the
      // entries, so we might spin on empty queues for a little while until the
      // count catches up.
      subqueue_t* queue = NULL;
      bool success = false;
      //for (size_t dequeue_slot = 0, i = 0; ;)
      for (size_t i = 0; ;)
      {
        //i = ++dequeue_slot & (QUEUE_COUNT - 1);
        i = dequeue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
        queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);  // Take ownership of the subqueue
        if (queue) {
          if (!queue->empty()) {                                                // Dequeue entry while we own the queue
            success = true;
            entry = queue->front();
            queue->pop();
            queues_[i].queue.store(queue, std::memory_order_release);           // Relinquish ownership
            break;                                                              // We have a entry
          } else {                                                              // Subqueue is empty, try the next one
            queues_[i].queue.store(queue, std::memory_order_release);           // Relinquish ownership
            if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;        // No entry, and no more left
          }
        } else {                                                                // queue is NULL, nothing to relinquish
          if (count_.load(std::memory_order_acquire) == 0) goto EMPTY;          // We failed to get a queue but there are no more entries left
        }
        if (success) break;
        if (count_.load(std::memory_order_acquire) == 0) break; 
        static const struct timespec ns = { 0, 1 };
        nanosleep(&ns, NULL);
      };
      // If we have a entry, decrement the queued entry count.
      count_.fetch_add(-1);
EMPTY:
      return success;
    }

    // Add a entry to the queue.
    // This method blocks until either the queue is full or a entry is added to
    // one of the subqueues. Note that the "queue is full" condition cannot be
    // checked atomically for all subqueues, so it's approximate, we try
    // several subqueues, if they are all full we give up.
    bool add(const T& entry) {
      // Preemptively increment the entry count: get() will spin on subqueues
      // as long as it thinks there is a entry to dequeue, but it will exit as
      // soon as the count is zero, so we want to avoid the situation when we
      // added a entry to a subqueue, have not incremented the count yet, but
      // get() exited with no entry. If this were to happen, the pool could be
      // deleted with entries still in the queue.
      count_.fetch_add(1);
      // Take ownership of a subqueue. The subqueue pointer is reset to NULL
      // while the calling thread owns the subqueue. When done, relinquish
      // the ownership by restoring the pointer.  The subqueue we got may be
      // full, in which case we try another subqueue, but don't loop forever
      // if all subqueues keep coming up full.
      subqueue_t* queue = NULL;
      bool success = false;
      //for (size_t enqueue_slot = 0, i = 0; ;)
      for (size_t i = 0; ;)
      {
        //i = ++enqueue_slot & (QUEUE_COUNT - 1);
        i = enqueue_slot_.fetch_add(1, std::memory_order_relaxed) & (QUEUE_COUNT - 1);
        queue = queues_[i].queue.exchange(nullptr, std::memory_order_acquire);    // Take ownership of the subqueue 
        if (queue) {
          success = true;
          queue->push(entry);                                                     // Enqueue entry while we own the queue 
          queues_[i].queue.store(queue, std::memory_order_release);               // Relinquish ownership
          return success;
        }
        static const struct timespec ns = { 0, 1 };
        nanosleep(&ns, NULL);
      };
      return success;
    }

  private:
    enum { QUEUE_COUNT = 16 };
    subqueue_ptr_t queues_[QUEUE_COUNT];
    std::atomic<int> count_;
    std::atomic<size_t> enqueue_slot_;
    std::atomic<size_t> dequeue_slot_;
};

#endif // CONCURRENT_QUEUE_H_
