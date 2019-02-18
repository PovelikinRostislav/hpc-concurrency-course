#ifndef INCLUDED_INTR_SHARED_PTR_H_
#define INCLUDED_INTR_SHARED_PTR_H_
#include <cstdlib>
#include <time.h>
#include <atomic>

// Intrusive reference-counted pointer:
// T - type the pointer points to
// U - T with reference counter
//     U must have the following member functions:
//     void AddRef() - atomically increment reference count by 1
//     bool DelRef() - atomically decrement reference count by 1, return true iff the counter dropped to 0
//     operator T&(), operator const T&() const - conversions to T (implicit if U is derived from T)
template <typename T, typename U = T> class intr_shared_ptr
{
  struct get_ptr {
    std::atomic<U*>& aptr;
    U* p;
    get_ptr(std::atomic<U*>& ptr) : aptr(ptr), p() {
      static const timespec ns = { 0, 1 };
      for (register int i = 0; aptr.load(std::memory_order_relaxed) == (U*)(locked) || (p = aptr.exchange((U*)(locked), std::memory_order_acquire)) == (U*)(locked); ++i) {
        if (i == 8) {
          i = 0;
          nanosleep(&ns, NULL);
        }
      }
    }
    ~get_ptr() {
      aptr.store(p, std::memory_order_release);
    }
    static const uintptr_t locked = uintptr_t(-1);
  };

  public:
  // Non-threadsafe shared pointer, used to hold non-zero reference counter to
  // safely dereference intr_shared_ptr.
  class shared_ptr {
    public:
    shared_ptr() : p_(nullptr) {
    }
    T& operator*() const { return *p_; }
    T* operator->() const { return p_; }
    shared_ptr(const shared_ptr& x) : p_(x.p_) {
      if (p_) p_->AddRef();
    }
    ~shared_ptr() {
      if (p_ && p_->DelRef()) {
        delete p_;
      }
    }
    explicit operator bool() const { return p_ != NULL; }
    shared_ptr& operator=(const shared_ptr& x) {
      if (this == &x) return *this;
      if (p_ && p_->DelRef()) {
        delete p_;
      }
      p_ = x.p_;
      if (p_) p_->AddRef();
      return *this;
    }
    bool operator==(const shared_ptr& rhs) const {
        return p_ == rhs.p_;
    }
    bool operator!=(const shared_ptr& rhs) const {
        return p_ != rhs.p_;
    }

    private:
    friend class intr_shared_ptr;
    explicit shared_ptr(U* p) : p_(p) {
      if (p_) p_->AddRef();
    }
    void reset(U* p) {
      if (p_ == p) return;
      if (p_ && p_->DelRef()) {
        delete p_;
      }
      p_ = p;
      if (p_) p_->AddRef();
    }
    U* p_;
  };

  explicit intr_shared_ptr(U* p = NULL) : p_(p) {
    if (p) p->AddRef();
  }
  explicit intr_shared_ptr(const shared_ptr& x) : p_() {
    if (x.p_) x.p_->AddRef();
    p_.store(x.p_, std::memory_order_relaxed);
  }
  explicit intr_shared_ptr(const intr_shared_ptr& x) : p_() {
    get_ptr px(x.p_);
    if (px.p) px.p->AddRef();
    p_.store(px.p, std::memory_order_relaxed);
  }
  ~intr_shared_ptr() {
    get_ptr p(p_);
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = NULL; // Destructor of p will copy this to p_
  }
  intr_shared_ptr& operator=(const intr_shared_ptr& x) {
    if (this == &x) return *this;
    // Beware of a deadlock:
    // get_ptr px(x.p_);
    // get_ptr p(p_);
    // Deadlock will happen if a = b; and b = a; are executed at the same time.
    U* pxp;
    {
      get_ptr px(x.p_);
      pxp = px.p;
      if (px.p) px.p->AddRef();
    }
    get_ptr p(p_);
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = pxp;  // Destructor of p will copy this to p_
    return *this;
  }
  void reset(U* x) {
    get_ptr p(p_);
    if (p.p == x) return;
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = x;  // Destructor of p will copy this to p_
    if (x) x->AddRef();
  }
  void reset(const shared_ptr& x) {
    get_ptr p(p_);
    if (p.p == x.p_) return;
    if (p.p && p.p->DelRef()) {
      delete p.p;
    }
    p.p = x.p_;  // Destructor of p will copy this to p_
    if (x.p_) x.p_->AddRef();
  }
  explicit operator bool() const { return p_.load(std::memory_order_relaxed) != NULL; }

  shared_ptr get() const {
    get_ptr p(p_);
    return shared_ptr(p.p);
  }

  bool compare_exchange_strong(shared_ptr& expected_ptr, const shared_ptr& new_ptr) {
    get_ptr p(p_);
    if (p.p == expected_ptr.p_) {
      if (p.p && p.p->DelRef()) {
        delete p.p;
      }
      p.p = new_ptr.p_;  // Destructor of p will copy this to p_
      if (p.p) p.p->AddRef();
      return true;
    } else {
      expected_ptr.reset(p.p);
      return false;
    }
  }

  bool compare_exchange_strong(shared_ptr& expected_ptr, U* new_ptr) {
    get_ptr p(p_);
    if (p.p == expected_ptr.p_) {
      if (p.p && p.p->DelRef()) {
        delete p.p;
      }
      p.p = new_ptr;  // Destructor of p will copy this to p_
      if (p.p) p.p->AddRef();
      return true;
    } else {
      expected_ptr.reset(p.p);
      return false;
    }
  }

  private:
  mutable std::atomic<U*> p_;
};
#endif // INCLUDED_INTR_SHARED_PTR_H_
