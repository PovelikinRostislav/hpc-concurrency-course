// atomic_queue is a simple lock-free queue of fixed maximum size.
//
// The queue is initialized with a range of memory to use and does not allocate
// any memory itself. 
//
// The queue can be specialized on any type, there is no requirement that the
// data type fits into atomic word.  The data type must be copyable using the
// standard copy constructor.
//
// The queue supports multiple enqueueing and dequeueing threads, the entries
// can be added and removed at the same time from multiple threads.  Note that
// the queue capacity is the upper limit of the number of entries that can be
// enqueued during the lifetime of the queue (even if they are quickly
// dequeued). Once the maximum number of entries is enqueued, the queue must be
// reset by calling Reset() before it can be used again. The queue does not
// reset itself automatically when it is empty.
// 
// The queue supports three methods for enqueueing entries:
//   Enqueue() is a general-purpose method
//   EnqueueRange() is a general-purpose method that enqueues several entries
//   at once and is preferable to calling Enqueue() multiple times, when
//   several values need to be enqueued
//   FastEnqueue() is much faster but can be used only if there is only one
//   enqueueing thread
// Use of either enqueue method does not restrict the choice of dequeue methods
// in any way. The two enqueueing methods should not be used at the same time.
// However, it is possible to use them on the same queue if it can be
// guaranteed that Enqueue() is never called from another thread when
// FastEnqueue() is called from the enqueueing thread.
//
// After the last entry has been enqueued, the user should call EndEnqueue().
// After EndEnqueue() is called, enqueueing of new entries is not allowed until
// Reset() is called.  Note that EndEnqueue() must be called before Reset() is
// called. It is not imperative to call EndEnqueue() as soon as the last entry
// is enqueued, but doing so can improve performance of dequeue operations.
//
// The queue supports only one method for dequeueing entries, Dequeue(). It is
// suitable for all use cases but is more efficient if the queue is not empry
// or if the queue is empty and EndEnqueue() was called.
//
// When Dequeue() is called on an empty queue, the special "empty value" is
// returned. This value can be specified when the queue is constructed; by
// default it is the default-initialized value, which is 0 for all integer
// types and NULL for all pointer types.
//
// Example of general use:
//   atomic_queue<int> queue;
//   queue.Enqueue(100); // thread 1
//   queue.Enqueue(200); // thread 2
//   queue.Dequeue();    // thread 1 gets 100
//   queue.Enqueue(300); // thread 3
//   queue.EndEnqueue();
//   queue.Dequeue();    // thread 2 gets 200
//   queue.Dequeue();    // thread 3 gets 300
//   queue.Dequeue();    // thread 1 gets 0
//   queue.Reset();
//
// Example of pre-primed queue:
//   atomic_queue<int> queue;
//   queue.FastEnqueue(100); // All enqueueing on thread 1
//   queue.FastEnqueue(200);
//   queue.FastEnqueue(300);
//   queue.EndEnqueue();
//   queue.Dequeue();    // thread 1 gets 100
//   queue.Dequeue();    // thread 2 gets 200
//   queue.Dequeue();    // thread 3 gets 300
//   queue.Dequeue();    // thread 1 gets 0
//   queue.Reset();
//

#ifndef ATOMIC_QUEUE_H_
#define ATOMIC_QUEUE_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <atomic>

namespace atomic_queue_utils {
template <typename T> struct IdentityMeta {
  typedef T type;
};
template<typename T> inline T implicit_cast(typename IdentityMeta<T>::type x) {
  return x;
}

template <typename To, typename From> struct bit_cast_helper {
  static To convert(const From& from) {
    To to;
    ::memcpy(&to, &from, sizeof(to));
    return to;
  }
};
template <typename T> struct bit_cast_helper<T, T> {
  static T convert(const T& from) { return from; }
};
template <typename To, typename From> inline To bit_cast(const From& from) {
  // Compile time assertion: sizeof(To) == sizeof(From)
  typedef char ToAndFromMustHaveSameSize[sizeof(To) == sizeof(From) ? 1 : -1];
  return bit_cast_helper<To, From>::convert(from);
}
} // namespace atomic_queue_utils

template <typename T> class atomic_queue {
    public:
    // Initialize empty queue. The queue is created with a given memory range
    // whose size is specified in bytes. The memory must be aligned
    // appropriately for the type T.
    // The caller owns the memory and must deallocate it at some point after the queue is deleted.
    // The special "empty value" is the default-initialized entry value by
    // default but can be overridden. This value is returned when empty queue
    // is dequeued.
    atomic_queue(void* memory, size_t bytes, const T& empty_value = T())
        : tail_(0),
          writer_tail_(0),
          head_(0),
          complete_(0),
          queue_(reinterpret_cast<T*>(memory)),
          max_length_(bytes/sizeof(T)),
          empty_value_(empty_value) {}

    // Reset the queue to its initial state.
    // Not thread-safe, should be called from single-threaded context.
    void Reset() {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
        StoreTail(WriterTail(), std::memory_order_relaxed);
        assert(complete_.exchange(0, std::memory_order_relaxed) == 1); // Reset called without EndEnqueue
    }
          
    // Add new entry at the tail of the queue.
    // Enqueue() fails iff the queue does not have enough memory for the new
    // entry or if the new entry equals the "empty value".
    bool Enqueue(const T& x) {
        assert(complete_.load(std::memory_order_relaxed) == 0); // Enqueue called after EndEnqueue
        // We cannot enqueue the "empty value" since returning it later would
        // make the queue appear empty.
        if (x == empty_value_) return false;
        // New entries are not released to the dequeueing threads as long as
        // there are multiple enqueuers all adding entries. It is, therefore,
        // possible to starve the dequeuers if the enqueuers are too fast. To
        // avoid this problem, we stall the enqueuers when there are too many
        // entries that are enqueued but not yet available to dequeuers.
        {
            size_t sleep_count = 0;
            const size_t max_enqueue_ahead = 32;
            while (LoadTail(std::memory_order_relaxed).tail > tail_.load(std::memory_order_relaxed) + max_enqueue_ahead) {
                NanoSleep(true, sleep_count);
            }
        }
        // Grab the slot at the tail of the queue.
        // Readers cannot use this slot yet, since we did not advance tail_.
        // Also increment the number of writers currently enqueueing.
        WriterTail wtail = IncrTail(WriterTail(1, 1), std::memory_order_acq_rel);
        // Check that there is enough space.
        if (wtail.tail > max_length_) {
            // Before exiting, always decrement the count of active writers.
            IncrTail(WriterTail(0, -1), std::memory_order_acq_rel);
            if (wtail.count == 0) {
                AdvanceTail(max_length_);
            }
            return false;
        }
        // Initialize new slot via copy constructor.
        size_t slot = wtail.tail - 1;
        new (atomic_queue_utils::implicit_cast<void*>(&queue_[slot])) T(x);
        // Decrement the number of writers currently enqueueing.
        wtail = IncrTail(WriterTail(0, -1), std::memory_order_acq_rel);
        // If this is the last writer currently enqueueing, advance tail_ and
        // make the new slots visible. This may publish slots created by
        // several writers at once, as long as this is the last writer from the
        // batch of writers that were enqueueing at the same time.
        // Release barrier ensures that initialization of queue_[slot] is
        // visible to all threads when this write succeeds.
        if (wtail.count == 0) {
            AdvanceTail(wtail.tail);
        }
        return true;
    }

    bool EnqueueRange(size_t count, const T* x) {
        assert(complete_.load(std::memory_order_relaxed) == 0); // Enqueue called after EndEnqueue
        // New entries are not released to the dequeueing threads as long as
        // there are multiple enqueuers all adding entries. It is, therefore,
        // possible to starve the dequeuers if the enqueuers are too fast. To
        // avoid this problem, we stall the enqueuers when there are too many
        // entries that are enqueued but not yet available to dequeuers.
        {
            size_t sleep_count = 0;
            const size_t max_enqueue_ahead = 32;
            while (LoadTail(std::memory_order_relaxed).tail > tail_.load(std::memory_order_relaxed) + max_enqueue_ahead) {
                NanoSleep(true, sleep_count);
            }
        }
        // Grab the slot at the tail of the queue.
        // Readers cannot use this slot yet, since we did not advance tail_.
        // Also increment the number of writers currently enqueueing.
        WriterTail wtail = IncrTail(WriterTail(count, 1), std::memory_order_acq_rel);
        // Check that there is enough space.
        if (wtail.tail > max_length_) {
            // Before exiting, always decrement the count of active writers.
            IncrTail(WriterTail(-count, -1), std::memory_order_acq_rel);
            return false;
        }
        // Initialize new slots via copy constructor.
        size_t slot = wtail.tail - count;
        for (size_t i = 0; i < count; ++i) {
            if (x[i] == empty_value_) return false;
            new (atomic_queue_utils::implicit_cast<void*>(&queue_[slot + i])) T(x[i]);
        }
        // Decrement the number of writers currently enqueueing.
        wtail = IncrTail(WriterTail(0, -1), std::memory_order_acq_rel);
        // If this is the last writer currently enqueueing, advance tail_ and
        // make the new slots visible. This may publish slots created by
        // several writers at once, as long as this is the last writer from the
        // batch of writers that were enqueueing at the same time.
        // Release barrier ensures that initialization of queue_[slot] is
        // visible to all threads when this write succeeds.
        if (wtail.count == 0) {
            AdvanceTail(wtail.tail);
        }
        return true;
    }

    // Fast non-thread-safe version of Enqueue(). Other threads may call
    // Dequeue() at the same time, but only one thread can call FastEnqueue().
    // FastEnqueue() fails iff the queue does not have enough memory for the
    // new entry or if the new entry equals the "empty value".
    bool FastEnqueue(const T& x) {
        assert(complete_.load(std::memory_order_relaxed) == 0); // FastEnqueue called after EndEnqueue
        // We cannot enqueue the "empty value" since returning it later would
        // make the queue appear empty.
        if (x == empty_value_) return false;
        // Grab the slot at the tail of the queue. This is the only writer to
        // tail_.
        // Readers cannot use this slot yet, since we did not advance tail_.
        size_t tail = tail_.load(std::memory_order_relaxed);
        // Check that there is enough space.
        if (tail > max_length_) return false;
        // Initialize new slot via copy constructor.
        new (atomic_queue_utils::implicit_cast<void*>(&queue_[tail])) T(x);
        // While FastEnqueue() does not use writer_tail_, subsequent calls to
        // Enqueue() will, so it needs to be correct.
        IncrTail(WriterTail(1, 0), std::memory_order_acq_rel);
        // Now advance tail_ and make the new slot visible, atomically.
        // Release barrier ensures that initialization of queue_[tail] is
        // visible to all threads when this write succeeds.
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    // This method should be called after the last call to Enqueue(). After
    // EndEnqueue() is called, no more entries can be added to the queue until
    // the next Reset() call.
    // Dequeue() can use the information that no more new entries are expected
    // to optimize its handling of empty queue. In particular, one should
    // expect that Dequeue() returns immediately if the queue is empty and no
    // more new entries are expected. However, if the queue is empty but
    // enqueueing is not done, Dequeue() can wait before returning to the
    // caller.
    void EndEnqueue() {
        complete_.store(1, std::memory_order_release);
    }

    // Dequeue() returns the first entry in the queue, or empty_value if the
    // queue is empty.
    T Dequeue() {
        // Atomically increment the head. This may be premature if we overshot
        // tail_ and claimed a non-existing slot.
        const size_t head = head_.fetch_add(1, std::memory_order_acq_rel);
        const size_t new_head = head + 1;
        size_t new_head1 = new_head;
        // Now we will either return the queue entry at the head, or attempt to
        // roll back the change of head_.
        size_t sleep_count = 0;
        do {
            // Check if any more entries are to be added. If not, current tail_
            // is at its final value.
            // Acquire barriers ensure that all reads happen in the program order.
            bool completed = complete_.load(std::memory_order_acquire) == 1;
            // Read the index of the current tail slot.
            // Acquire barrier matches the Release barrier in Enqueue() and ensures that
            // if we read queue_[tail_] it happens after we read and verify tail_.
            // If there queue is not empty, we can dequeue the head element.
            if (head < tail_.load(std::memory_order_acquire)) return queue_[head];
            // Not enough entries in the queue and no new ones are coming -
            // queue is empty.
            if (completed) return empty_value_;
            // If the queue is empty, we should decrement head_, but multiple
            // readers can be in the same situation, so each one must undo only
            // its own incrementing of head_. It is possible that while we wait
            // for other readers, new entries will be enqueued and the slot
            // queue_[head] becomes valid, in which case we will return it.
        } while (head_.compare_exchange_strong(new_head1, head, std::memory_order_relaxed) == false && NanoSleep(true, sleep_count));
        return empty_value_;
    }

    // Check if the queue is empty.
    // Note that the result of this function may become incorrect even before
    // the function returns, since other threads can add or remove entries to
    // the queue at any time.
    bool Empty() const {
        // Read the index of the current tail slot.
        size_t tail = tail_.load(std::memory_order_relaxed);
        // Read the index of the current head slot, after reading tail slot.
        size_t head = head_.load(std::memory_order_acquire);
        return head >= tail;
    }

    // Return the length of the queue.
    // This function returns approximate number of elements in the queue, since
    // reading head and tail positions are not done atomically as a pair.
    size_t Length() const {
        // Read the index of the current tail slot.
        size_t tail = tail_.load(std::memory_order_relaxed);
        // Read the index of the current head slot, after reading tail slot.
        size_t head = head_.load(std::memory_order_acquire);
        if (head >= tail) return 0;  // head_ can overshoot tail_ on empty queue, we may decide to not correct this for performance reasons
        else return tail - head;
    }

    // Check if EndEnqueue() was called.
    bool Finalized() const {
        return complete_.load(std::memory_order_acquire);
    }

    // Return the maximum length of the queue.
    size_t Capacity() const { return max_length_; }

    // Return the "empty value" as defined in the constructor.
    T EmptyValue() const { return empty_value_; }

    private:
    std::atomic<size_t> tail_;
    // WriterTail packs the tail value (index into the queue array) and the
    // number of threads currently enqueueing into one atomic word.
    // This is so we can atomically increment both the tail and the reference count at once.
    // The way the bits are split between the two values determines the upper
    // limits on the queue: there cannot be more threads enqueueing at once
    // than writer_tail_.count can store, and the queue size is limited by the
    // maximum number that can be stored in writer_tail_.tail.
    // The MAX_THREAD_COUNT_BITS value below controls the split:
    // MAX_THREAD_COUNT_BITS    maximum threads   maximum queue size
    //         16                    64K                16T
    //         24                     4M                 1T
    //         32                     4G                 4G
    enum { MAX_THREAD_COUNT_BITS = 32 };
    // The order of the fields in the pached structure matters because we want
    // to increment them both atomically, so we are concerned about carryover.
#ifdef __LITTLE_ENDIAN
    struct WriterTail {
        size_t tail : sizeof(size_t)*8-MAX_THREAD_COUNT_BITS;
        size_t count : MAX_THREAD_COUNT_BITS;
        WriterTail() noexcept(true) : tail(), count() {}
        WriterTail(unsigned int t, unsigned int c) : tail(t), count(c) {}
    };
#else 
    struct WriterTail {
        size_t count : MAX_THREAD_COUNT_BITS;
        size_t tail : sizeof(size_t)*8-MAX_THREAD_COUNT_BITS;
        WriterTail() noexcept(true) : count(), tail() {}
        WriterTail(unsigned int t, unsigned int c) : count(c), tail(t) {}
    };
#endif // __LITTLE_ENDIAN 
    WriterTail LoadTail(std::memory_order sync) {
      static_assert(sizeof(WriterTail) == sizeof(size_t), "WriterTail has wrong size");
      return atomic_queue_utils::bit_cast<WriterTail>(writer_tail_.load(sync));
    }
    void StoreTail(WriterTail val, std::memory_order sync) {
      writer_tail_.store(atomic_queue_utils::bit_cast<size_t>(val), sync);
    }
    WriterTail IncrTail(WriterTail inc, std::memory_order sync) {
      return atomic_queue_utils::bit_cast<WriterTail>(writer_tail_.fetch_add(atomic_queue_utils::bit_cast<size_t>(inc), sync) + atomic_queue_utils::bit_cast<size_t>(inc));
    }
    std::atomic<size_t> writer_tail_;
    std::atomic<size_t> head_;
    std::atomic<size_t> complete_;
    T* const queue_;
    const size_t max_length_;
    T empty_value_;

    // After the last enqueuer advances writer_tail_ and is ready to exit, it
    // is possible that another enqueuer advanced tail_ after the count was
    // read but before we got to incrementing tail_. In this case, tail_ will
    // be larger than the value of writer_tail_.tail we have here, and we do
    // not need to do anything (the other enqueuer already advanced tail_ to
    // account for its own additions to the queue as well as ours). However,
    // this test must be done under lock (we do not know what the right old
    // value of tail_ for CompareAndSwap would be).
    void AdvanceTail(size_t new_tail) {
        FastLock L(lock_);
                
        if (tail_.load(std::memory_order_relaxed) < new_tail) {
            tail_.store(new_tail, std::memory_order_release);
        }
    }

    // A fast minimalistic spinlock class.
    std::atomic<int> lock_;
    class FastLock {
        public:
        // Atomically store 1 in the lock. If the previous value was 0, we got
        // the lock and can proceed. Otherwise we must wait.
        FastLock(std::atomic<int>& lock) : lock_(lock) { 
            while (lock_.exchange(1, std::memory_order_acquire) == 1) {
                if (lock_.exchange(1, std::memory_order_acquire) == 0) return; // 2
                if (lock_.exchange(1, std::memory_order_acquire) == 0) return; // 3
                if (lock_.exchange(1, std::memory_order_acquire) == 0) return; // 4
                if (lock_.exchange(1, std::memory_order_acquire) == 0) return; // 5
                if (lock_.exchange(1, std::memory_order_acquire) == 0) return; // 6
                if (lock_.exchange(1, std::memory_order_acquire) == 0) return; // 7
                if (lock_.exchange(1, std::memory_order_acquire) == 0) return; // 8
                static const struct timespec req = { 0, 1 };
                nanosleep(&req, NULL);
            }
        }

        // Atomically store 0 in the lock. If another thread is waiting, it
        // will be able to proceed now.
        ~FastLock() { lock_.store(0, std::memory_order_release); }

        private:
        std::atomic<int>& lock_;
    };

    // Sleep for a very short time.
    // The argument is returned as the return value, this allows using the
    // function inside loop conditions so the thread sleeps only if the loop
    // does not terminate:
    //   size_t sleep_count = 0;
    //   do { ... } while (!success && NanoSleep(true, sleep_count));
    static bool NanoSleep(bool x, size_t& sleep_count) {
        static const struct timespec req = { 0, 1 };
        if (++sleep_count == 8) {
            nanosleep(&req, NULL);
            sleep_count = 0;
        }
        return x;
    }
};

#endif // ATOMIC_QUEUE_H_
