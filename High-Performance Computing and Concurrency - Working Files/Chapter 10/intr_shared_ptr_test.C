#include <intr_shared_ptr.h>

#include <atomic>

#include <gtest/gtest.h>

using namespace std;

struct A {
  int i;
  A(int i = 0) : i(i) {}
};

static long B_count = 0;
struct B : public A {
  B(int i = 0) : A(i), ref_cnt_(0) {
    ++B_count;
  }
  ~B() {
      --B_count; 
  }
  B(const B& x) = delete;
  B& operator=(const B& x) = delete;
  atomic<unsigned long> ref_cnt_;
  void AddRef() { ref_cnt_.fetch_add(1); }
  bool DelRef() { return ref_cnt_.fetch_sub(1) == 1; }
};

TEST(PtrTestAB, InitNULL) {
  intr_shared_ptr<A, B> p;
  EXPECT_FALSE(p);
}

TEST(PtrTestAB, Init) {
  {
    intr_shared_ptr<A, B> p(new B(42));
    EXPECT_TRUE(p);
    EXPECT_EQ(2u, static_cast<B&>(*p.get()).ref_cnt_);  // Extra count for temp shared_ptr
    EXPECT_EQ(1, B_count);
    EXPECT_EQ(42, p.get()->i);
  }
  EXPECT_EQ(0, B_count);
}

TEST(PtrTestAB, SharedPtrInitNULL) {
  intr_shared_ptr<A, B> p;
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  EXPECT_FALSE(p);
  EXPECT_FALSE(p1);
}

TEST(PtrTestAB, SharedPtrInit) {
  {
    intr_shared_ptr<A, B> p(new B(42));
    intr_shared_ptr<A, B>::shared_ptr p1(p.get());
    EXPECT_TRUE(p);
    EXPECT_TRUE(p1);
    EXPECT_EQ(3u, static_cast<B&>(*p.get()).ref_cnt_);  // Extra count for temp shared_ptr
    EXPECT_EQ(1, B_count);
    EXPECT_EQ(42, p.get()->i);
    EXPECT_EQ(42, p1->i);
  }
  EXPECT_EQ(0, B_count);
}

TEST(PtrTestAB, Copy) {
  {
    intr_shared_ptr<A, B> p(new B(42));
    {
      intr_shared_ptr<A, B> q(p);
      EXPECT_TRUE(q);
      EXPECT_EQ(3u, static_cast<B&>(*q.get()).ref_cnt_);
      EXPECT_EQ(1, B_count);
      EXPECT_EQ(42, q.get()->i);
    }
    EXPECT_TRUE(p);
    EXPECT_EQ(2u, static_cast<B&>(*p.get()).ref_cnt_);
    EXPECT_EQ(1, B_count);
    EXPECT_EQ(42, p.get()->i);
  }
  EXPECT_EQ(0, B_count);
}

TEST(PtrTestAB, AssignDelete) {
  {
    intr_shared_ptr<A, B> p(new B(42));
    intr_shared_ptr<A, B> q(new B(7));
    EXPECT_TRUE(p);
    EXPECT_TRUE(q);
    EXPECT_EQ(2u, static_cast<B&>(*p.get()).ref_cnt_);
    EXPECT_EQ(2u, static_cast<B&>(*q.get()).ref_cnt_);
    EXPECT_EQ(2, B_count);
    EXPECT_EQ(42, p.get()->i);
    EXPECT_EQ(7, q.get()->i);
    
    p = q;
    EXPECT_TRUE(p);
    EXPECT_TRUE(q);
    EXPECT_EQ(3u, static_cast<B&>(*p.get()).ref_cnt_);
    EXPECT_EQ(3u, static_cast<B&>(*q.get()).ref_cnt_);
    EXPECT_EQ(1, B_count);
    EXPECT_EQ(7, p.get()->i);
    EXPECT_EQ(7, q.get()->i);
  }
  EXPECT_EQ(0, B_count);
}

TEST(PtrTestAB, Assign) {
  {
    intr_shared_ptr<A, B> p(new B(42));
    intr_shared_ptr<A, B> r(p);
    {
      intr_shared_ptr<A, B> q(new B(7));
      EXPECT_TRUE(p);
      EXPECT_TRUE(q);
      EXPECT_TRUE(r);
      EXPECT_EQ(3u, static_cast<B&>(*p.get()).ref_cnt_);
      EXPECT_EQ(2u, static_cast<B&>(*q.get()).ref_cnt_);
      EXPECT_EQ(3u, static_cast<B&>(*r.get()).ref_cnt_);
      EXPECT_EQ(2, B_count);
      EXPECT_EQ(42, p.get()->i);
      EXPECT_EQ(7, q.get()->i);
      EXPECT_EQ(42, r.get()->i);

      p = q;
      EXPECT_TRUE(p);
      EXPECT_TRUE(q);
      EXPECT_TRUE(r);
      EXPECT_EQ(3u, static_cast<B&>(*p.get()).ref_cnt_);
      EXPECT_EQ(3u, static_cast<B&>(*q.get()).ref_cnt_);
      EXPECT_EQ(2u, static_cast<B&>(*r.get()).ref_cnt_);
      EXPECT_EQ(2, B_count);
      EXPECT_EQ(7, p.get()->i);
      EXPECT_EQ(7, q.get()->i);
      EXPECT_EQ(42, r.get()->i);
    }
    EXPECT_TRUE(p);
    EXPECT_TRUE(r);
    EXPECT_EQ(2u, static_cast<B&>(*p.get()).ref_cnt_);
    EXPECT_EQ(2u, static_cast<B&>(*r.get()).ref_cnt_);
    EXPECT_EQ(2, B_count);
    EXPECT_EQ(7, p.get()->i);
    EXPECT_EQ(42, r.get()->i);
  }
  EXPECT_EQ(0, B_count);
}

TEST(PtrTestAB, CASsuccess) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  B* p2 = new B(7);
  ASSERT_EQ(2, B_count);
  EXPECT_TRUE(p.compare_exchange_strong(p1, p2));
  EXPECT_EQ(2, B_count);
  EXPECT_EQ(42, p1->i);
  EXPECT_EQ(7, p.get()->i);
  EXPECT_EQ(1u, static_cast<B&>(*p1).ref_cnt_); // p1
  EXPECT_EQ(1u, static_cast<B&>(*p2).ref_cnt_); // p
  EXPECT_EQ(2u, static_cast<B&>(*p.get()).ref_cnt_);    // p, temporary
}

TEST(PtrTestAB, CASsuccessNULL) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  ASSERT_EQ(1, B_count);
  EXPECT_TRUE(p.compare_exchange_strong(p1, NULL));
  EXPECT_EQ(1, B_count);
  EXPECT_EQ(42, p1->i);
  EXPECT_FALSE(bool(p));
  EXPECT_EQ(1u, static_cast<B&>(*p1).ref_cnt_); // p1
}

TEST(PtrTestAB, CASfail) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  intr_shared_ptr<A, B> q(new B(7));
  intr_shared_ptr<A, B>::shared_ptr q1(q.get());
  ASSERT_EQ(2, B_count);
  EXPECT_FALSE(p.compare_exchange_strong(q1, NULL));
  EXPECT_EQ(2, B_count);
  EXPECT_EQ(42, q1->i);
  EXPECT_EQ(42, p.get()->i);
  EXPECT_EQ(3u, static_cast<B&>(*p1).ref_cnt_); // p, p1, q1
  EXPECT_EQ(4u, static_cast<B&>(*p.get()).ref_cnt_);    // p, p1, q1, temporary
  EXPECT_EQ(2u, static_cast<B&>(*q.get()).ref_cnt_);    // q, temporary
}

TEST(PtrTestAB, Reset) {
  intr_shared_ptr<A, B> p(new B(42));
  B* p2 = new B(7);
  ASSERT_EQ(2, B_count);
  p.reset(p2);
  EXPECT_EQ(1, B_count);
  EXPECT_EQ(7, p.get()->i);
  EXPECT_EQ(1u, static_cast<B&>(*p2).ref_cnt_); // p
  EXPECT_EQ(2u, static_cast<B&>(*p.get()).ref_cnt_);    // p, temporary
}

TEST(PtrTestAB, ResetNULL) {
  intr_shared_ptr<A, B> p(new B(42));
  ASSERT_EQ(1, B_count);
  p.reset(NULL);
  EXPECT_EQ(0, B_count);
  EXPECT_FALSE(bool(p));
}

TEST(PtrTestAB, CASsuccessSharedPtr) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  intr_shared_ptr<A, B> q(new B(7));
  intr_shared_ptr<A, B>::shared_ptr p2(q.get());
  ASSERT_EQ(2, B_count);
  EXPECT_TRUE(p.compare_exchange_strong(p1, p2));
  EXPECT_EQ(2, B_count);
  EXPECT_EQ(42, p1->i);
  EXPECT_EQ(7, p.get()->i);
  EXPECT_EQ(1u, static_cast<B&>(*p1).ref_cnt_); // p1
  EXPECT_EQ(3u, static_cast<B&>(*p2).ref_cnt_); // p, q, p2
  EXPECT_EQ(4u, static_cast<B&>(*p.get()).ref_cnt_);    // p, q, p2, temporary
}

TEST(PtrTestAB, CASsuccessNULLSharedPtr) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  intr_shared_ptr<A, B> q(NULL);
  intr_shared_ptr<A, B>::shared_ptr p2(q.get());
  ASSERT_EQ(1, B_count);
  EXPECT_TRUE(p.compare_exchange_strong(p1, p2));
  EXPECT_EQ(1, B_count);
  EXPECT_EQ(42, p1->i);
  EXPECT_FALSE(bool(p));
  EXPECT_EQ(1u, static_cast<B&>(*p1).ref_cnt_); // p1
}

TEST(PtrTestAB, CASfailSharedPtr) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  intr_shared_ptr<A, B> q(new B(7));
  intr_shared_ptr<A, B>::shared_ptr q1(q.get());
  ASSERT_EQ(2, B_count);
  EXPECT_FALSE(p.compare_exchange_strong(q1, q1));
  EXPECT_EQ(2, B_count);
  EXPECT_EQ(42, q1->i);
  EXPECT_EQ(42, p.get()->i);
  EXPECT_EQ(3u, static_cast<B&>(*p1).ref_cnt_); // p, p1, q1
  EXPECT_EQ(4u, static_cast<B&>(*p.get()).ref_cnt_);    // p, p1, q1, temporary
  EXPECT_EQ(2u, static_cast<B&>(*q.get()).ref_cnt_);    // q, temporary
}

TEST(PtrTestAB, ResetSharedPtr) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B> q(new B(7));
  intr_shared_ptr<A, B>::shared_ptr q1(q.get());
  ASSERT_EQ(2, B_count);
  p.reset(q1);
  EXPECT_EQ(1, B_count);
  EXPECT_EQ(7, p.get()->i);
  EXPECT_EQ(3u, static_cast<B&>(*q1).ref_cnt_); // p, q, q1
  EXPECT_EQ(4u, static_cast<B&>(*p.get()).ref_cnt_);    // p, q, q1, temporary
}

TEST(PtrTestAB, SharedPtrAssign) {
  {
    intr_shared_ptr<A, B> p(new B(42));
    intr_shared_ptr<A, B>::shared_ptr p1(p.get());
    intr_shared_ptr<A, B> q(new B(7));
    intr_shared_ptr<A, B>::shared_ptr q1(q.get());
    p1 = q1;
    EXPECT_TRUE(p1);
    EXPECT_TRUE(q1);
    EXPECT_EQ(2u, static_cast<B&>(*p.get()).ref_cnt_);  // Extra count for temp shared_ptr
    EXPECT_EQ(4u, static_cast<B&>(*q.get()).ref_cnt_);  // Extra count for temp shared_ptr
    EXPECT_EQ(2, B_count);
    EXPECT_EQ(42, p.get()->i);
    EXPECT_EQ(7, p1->i);
  }
  EXPECT_EQ(0, B_count);
}

TEST(PtrTestAB, SharedPtrCompare) {
  intr_shared_ptr<A, B> p(new B(42));
  intr_shared_ptr<A, B>::shared_ptr p1(p.get());
  intr_shared_ptr<A, B>::shared_ptr p2(p.get());
  intr_shared_ptr<A, B> q(new B(7));
  intr_shared_ptr<A, B>::shared_ptr q1(q.get());
  EXPECT_NE(p1, q1);
  EXPECT_EQ(p1, p2);
  p1 = q1;
  EXPECT_EQ(p1, q1);
}

static long C_count = 0;
struct C {
  int i;
  C(int i = 0) : i(i), ref_cnt_(0) {
    ++C_count;
  }
  ~C() { --C_count; }
  C(const C& x) = delete;
  C& operator=(const C& x) = delete;
  atomic<unsigned long> ref_cnt_;
  void AddRef() { ref_cnt_.fetch_add(1); }
  bool DelRef() { return ref_cnt_.fetch_sub(1) == 1; }
};

TEST(PtrTestC, Init) {
  {
    intr_shared_ptr<C> p(new C(42));
    EXPECT_TRUE(p);
    EXPECT_EQ(2u, static_cast<C&>(*p.get()).ref_cnt_);
    EXPECT_EQ(1, C_count);
    EXPECT_EQ(42, p.get()->i);
  }
  EXPECT_EQ(0, C_count);
}
