#include <atomic-forward-list.h>

#include <gtest/gtest.h>

#include <iomanip>
#include <iostream>
using namespace std;

static long C_count = 0;
struct C {
  size_t x;
  C() : x() { ++C_count; }
  C(size_t x) : x(x) { ++C_count; }
  C(const C& c) : x(c.x) { ++C_count; }
  ~C() { --C_count; }
};
bool operator==(const C& a, const C& b) { return a.x == b.x; }
typedef atomic_forward_list<C> list_t;
typedef list_t::iterator iterator_t;

TEST(AtomicForwardListTest, Construct) {
  ASSERT_EQ(0, C_count);
  list_t l;
  EXPECT_TRUE(l.empty());
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, PushFront) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  EXPECT_FALSE(l.empty());
  EXPECT_EQ(1, C_count);
}

TEST(AtomicForwardListTest, DtorCleans) {
  ASSERT_EQ(0, C_count);
  {
    list_t l;
    l.push_front(C(1));
  }
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, PushFront2) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));
  EXPECT_FALSE(l.empty());
  EXPECT_EQ(2, C_count);
}

TEST(AtomicForwardListTest, DtorCleans2) {
  ASSERT_EQ(0, C_count);
  {
    list_t l;
    l.push_front(C(1));
    l.push_front(C(2));
  }
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, PopFront) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  {
    C c(0);
    EXPECT_TRUE(l.pop_front(c));
    EXPECT_EQ(1u, c.x);
  }
  EXPECT_TRUE(l.empty());
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, Clear) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));
  l.clear();
  EXPECT_TRUE(l.empty());
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, ClearPushClear) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));
  l.clear();
  l.push_front(C(1));
  l.push_front(C(2));
  l.push_front(C(3));
  EXPECT_FALSE(l.empty());
  EXPECT_EQ(3, C_count);
  l.clear();
  EXPECT_TRUE(l.empty());
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, BeforeBegin) {
  ASSERT_EQ(0, C_count);
  list_t l;
  iterator_t it = l.before_begin();
  EXPECT_TRUE(bool(it));
}

TEST(AtomicForwardListTest, Begin) {
  ASSERT_EQ(0, C_count);
  list_t l;
  iterator_t it = l.begin();
  EXPECT_FALSE(bool(it));
}

TEST(AtomicForwardListTest, Begin1) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  iterator_t it = l.begin();
  EXPECT_TRUE(bool(it));
  EXPECT_EQ(1u, it->x);
  EXPECT_EQ(1u, (*it).x);
}

TEST(AtomicForwardListTest, End) {
  ASSERT_EQ(0, C_count);
  list_t l;
  iterator_t it = l.end();
  EXPECT_FALSE(bool(it));
}

TEST(AtomicForwardListTest, End1) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  iterator_t it = l.end();
  EXPECT_FALSE(bool(it));
}

TEST(AtomicForwardListTest, IteratorIncr) {
  ASSERT_EQ(0, C_count);
  list_t l;
  iterator_t it = l.before_begin();
  ++it;
  EXPECT_FALSE(bool(it));
}

TEST(AtomicForwardListTest, IteratorIncr1) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  iterator_t it = l.before_begin();
  ++it;
  EXPECT_TRUE(bool(it));
  EXPECT_EQ(1u, it->x);
  EXPECT_EQ(1u, (*it).x);
  ++it;
  EXPECT_FALSE(bool(it));
}

TEST(AtomicForwardListTest, IteratorCompare) {
  ASSERT_EQ(0, C_count);
  list_t l;
  iterator_t it1 = l.before_begin();
  EXPECT_EQ(it1, l.before_begin());
  iterator_t it2 = l.begin();
  EXPECT_NE(it1, it2);
  iterator_t it3 = l.end();
  EXPECT_NE(it1, it3);
  EXPECT_EQ(it2, it3);
  ++it1;
  EXPECT_EQ(it1, l.begin());
  EXPECT_EQ(it1, it2);
  EXPECT_EQ(it1, it3);
}

TEST(AtomicForwardListTest, IteratorCompare1) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  iterator_t it1 = l.before_begin();
  EXPECT_EQ(it1, l.before_begin());
  iterator_t it2 = l.begin();
  EXPECT_NE(it1, it2);
  iterator_t it3 = l.end();
  EXPECT_NE(it1, it3);
  EXPECT_NE(it2, it3);
  ++it1;
  EXPECT_EQ(it1, l.begin());
  EXPECT_EQ(it1, it2);
  EXPECT_NE(it1, it3);
  ++it1;
  EXPECT_EQ(it1, l.end());
  EXPECT_EQ(it1, it3);
  EXPECT_NE(it1, it2);
}

TEST(AtomicForwardListTest, IteratorIncr2) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));
  iterator_t it = l.before_begin();
  EXPECT_NE(it, l.end());

  EXPECT_EQ(l.begin(), ++it);
  EXPECT_TRUE(bool(it));
  EXPECT_EQ(2u, it->x);
  EXPECT_EQ(2u, (*it).x);
  EXPECT_EQ(it, l.begin());
  EXPECT_NE(it, l.end());

  EXPECT_NE(l.begin(), ++it);
  EXPECT_TRUE(bool(it));
  EXPECT_EQ(1u, it->x);
  EXPECT_EQ(1u, (*it).x);
  EXPECT_NE(it, l.end());

  EXPECT_EQ(l.end(), ++it);
  EXPECT_FALSE(bool(it));
  EXPECT_EQ(it, l.end());
}

TEST(AtomicForwardListTest, IteratorPreIncr2) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));
  iterator_t it = l.before_begin();
  EXPECT_NE(it, l.end());

  EXPECT_EQ(l.before_begin(), it++);
  EXPECT_TRUE(bool(it));
  EXPECT_EQ(2u, it->x);
  EXPECT_EQ(2u, (*it).x);
  EXPECT_EQ(it, l.begin());
  EXPECT_NE(it, l.end());

  EXPECT_EQ(l.begin(), it++);
  EXPECT_TRUE(bool(it));
  EXPECT_EQ(1u, it->x);
  EXPECT_EQ(1u, (*it).x);
  EXPECT_NE(it, l.end());

  EXPECT_NE(l.end(), it++);
  EXPECT_FALSE(bool(it));
  EXPECT_EQ(it, l.end());
}

TEST(AtomicForwardListTest, InsertAfterHead) {
  ASSERT_EQ(0, C_count);
  {
    list_t l;
    iterator_t it1 = l.before_begin();
    iterator_t it2 = l.insert_after(it1, C(1));
    EXPECT_TRUE(it2);
    EXPECT_FALSE(l.empty());
    EXPECT_EQ(1, C_count);
    EXPECT_EQ(1u, l.begin()->x);
    EXPECT_EQ(l.begin(), it2);
  }
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, InsertAfterBegin) {
  ASSERT_EQ(0, C_count);
  {
    list_t l;
    l.push_front(C(1));
    iterator_t it1 = l.begin();
    iterator_t it2 = l.insert_after(it1, C(2));
    EXPECT_TRUE(it2);
    EXPECT_EQ(2, C_count);
    EXPECT_EQ(1u, l.begin()->x);
    EXPECT_EQ(2u, it2->x);
    ++it1;
    EXPECT_EQ(it1, it2);
  }
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, EraseAfterBeforeBegin) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));

  iterator_t it = l.erase_after(l.before_begin());
  EXPECT_EQ(l.begin(), it);
  EXPECT_EQ(1u, it->x);
  EXPECT_FALSE(l.empty());
  EXPECT_EQ(1, C_count);

  it = l.erase_after(l.before_begin());
  EXPECT_EQ(l.begin(), it);
  EXPECT_EQ(l.end(), it);
  EXPECT_TRUE(l.empty());
  EXPECT_EQ(0, C_count);
}

TEST(AtomicForwardListTest, EraseAfterBegin) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));

  iterator_t it = l.erase_after(l.begin());
  EXPECT_EQ(l.end(), it);
  EXPECT_FALSE(l.empty());
  EXPECT_EQ(1, C_count);

  it = l.erase_after(l.begin());
  EXPECT_NE(l.begin(), it);
  EXPECT_EQ(l.end(), it);
  EXPECT_FALSE(l.empty());
  EXPECT_EQ(1, C_count);
}

TEST(AtomicForwardListTest, Find) {
  ASSERT_EQ(0, C_count);
  list_t l;
  l.push_front(C(1));
  l.push_front(C(2));
  l.push_front(C(3));
  iterator_t it1 = l.begin();

  iterator_t it = l.find(3);
  EXPECT_EQ(3u, it->x);
  EXPECT_EQ(l.begin(), it);

  it = l.find(2);
  EXPECT_EQ(2u, it->x);
  EXPECT_EQ(++it1, it);

  it = l.find(1);
  EXPECT_EQ(1u, it->x);
  EXPECT_EQ(++it1, it);

  it = l.find(0);
  EXPECT_EQ(++it1, it);
  EXPECT_EQ(l.end(), it);
}
