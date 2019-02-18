#include <intr_shared_ptr.h>

#include <cassert>
#include <atomic>
#include <thread>
#include <vector>

#define NDEBUG 

using namespace std;

struct A {
  int i;
  A(int i = 0) : i(i) {}
};

static unsigned long B_count = 0;
struct B : public A {
  B(int i = 0) : A(i), ref_cnt_(0) {
    ++B_count;
  }
  ~B() { --B_count; }
  B(const B& x) = delete;
  B& operator=(const B& x) = delete;
  atomic<unsigned long> ref_cnt_;
  void AddRef() { ref_cnt_.fetch_add(1, std::memory_order_acq_rel); }
  bool DelRef() { return ref_cnt_.fetch_sub(1, std::memory_order_acq_rel) == 1; }
  // TSAN bug demo (also try TSAN_OPTIONS="force_seq_cst_atomics=1" ./intr_shared_ptr_tsan)
  //void AddRef() { ref_cnt_.fetch_add(1, std::memory_order_relaxed); }
  //bool DelRef() { return ref_cnt_.fetch_sub(1, std::memory_order_relaxed) == 1; }
};

void copy_ptr(const intr_shared_ptr<A, B>& p, size_t N, size_t M) {
  for (size_t i = 0; i < N; ++i) {
    vector<intr_shared_ptr<A, B> > v;
    for (size_t j = 0; j < M; ++j) {
      v.push_back(p);
    }
  }
}

void Test1() {
  {
    intr_shared_ptr<A, B> p(new B(42));
    {
      thread t1(copy_ptr, p, 100, 10000);
      thread t2(copy_ptr, p, 100, 10000);
      t1.join();
      t2.join();
    }
    assert(p);
    assert(2 == static_cast<B&>(*p.get()).ref_cnt_);
    assert(1 == B_count);
    assert(42 == p.get()->i);
  }
  assert(0 == B_count);
}

void cross_assign(intr_shared_ptr<A, B>* p, intr_shared_ptr<A, B>* q, size_t N) {
  for (size_t i = 0; i < N; ++i) {
    *p = *q;
  }
}

void Test2() {
  {
    intr_shared_ptr<A, B> p(new B(42));
    intr_shared_ptr<A, B> q(new B(7));
    intr_shared_ptr<A, B> r(new B(314));
    {
      thread t1(cross_assign, &p, &q, 10000);
      thread t2(cross_assign, &q, &r, 10000);
      thread t3(cross_assign, &r, &p, 10000);
      t1.join();
      t2.join();
      t3.join();
    }
    assert(p);
    assert(q);
    assert(r);
    assert(1 == B_count);
  }
  assert(0 == B_count);
}

int main() {
  Test1();
  Test2();
}
