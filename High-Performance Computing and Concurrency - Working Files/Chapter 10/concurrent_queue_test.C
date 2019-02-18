#include <concurrent_queue.h>

#include <stdlib.h>
#include <string.h>
#include <new>

#include <gtest/gtest.h>

/*
typedef int entry_t;
*/
struct entry_t {
  entry_t(int x = 0) : x(x) { ::memset(pad, 0, sizeof(pad)); }
  bool operator==(const entry_t& rhs) const { return x == rhs.x; }
  int x;
  int pad[128];
};
bool operator==(int x, const entry_t& y) { return x == y.x; }

class SubqueueTest : public ::testing::Test {
    public:
    typedef subqueue<entry_t> subqueue_t;
    enum { capacity = 4 };
    SubqueueTest() : queue(new(::malloc(subqueue_t::memsize(capacity))) subqueue_t(capacity)) {}
    ~SubqueueTest() {
        EXPECT_EQ(capacity, queue->capacity());
        ::free(queue);
    }

    subqueue_t* queue;
};

TEST_F(SubqueueTest, Construct) {
    EXPECT_EQ(0u, queue->size());
    EXPECT_EQ(0u, queue->begin());
}

TEST_F(SubqueueTest, AddOne) {
    EXPECT_TRUE(queue->add(1));
    EXPECT_EQ(1u, queue->size());
    EXPECT_EQ(0u, queue->begin());
}

TEST_F(SubqueueTest, GetOne) {
    EXPECT_TRUE(queue->add(1));
    entry_t x = 42;
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(1, x);
    EXPECT_EQ(0u, queue->size());
    EXPECT_EQ(1u, queue->begin());
}

TEST_F(SubqueueTest, GetEmpty) {
    entry_t x = 42;
    EXPECT_FALSE(queue->get(x));
    EXPECT_EQ(42, x);
    EXPECT_EQ(0u, queue->size());
    EXPECT_EQ(0u, queue->begin());
}

TEST_F(SubqueueTest, AddOneGetTwo) {
    EXPECT_TRUE(queue->add(1));
    entry_t x = 42;
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(1, x);
    x = 42;
    EXPECT_FALSE(queue->get(x));
    EXPECT_EQ(42, x);
    EXPECT_EQ(0u, queue->size());
    EXPECT_EQ(1u, queue->begin());
}

TEST_F(SubqueueTest, AddGet) {
    EXPECT_TRUE(queue->add(1));
    EXPECT_TRUE(queue->add(2));
    entry_t x = 42;
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(1, x);
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(2, x);
    EXPECT_FALSE(queue->get(x));
    EXPECT_EQ(2, x);
    EXPECT_EQ(0u, queue->size());
    EXPECT_EQ(2u, queue->begin());
}

TEST_F(SubqueueTest, LoopOne) {
    for (size_t i = 0; i < 3*capacity; ++i) {
        EXPECT_TRUE(queue->add(i)) << "i=" << i;
        EXPECT_EQ(1u, queue->size()) << "i=" << i;
        EXPECT_EQ(i % capacity, queue->begin()) << "i=" << i;
        entry_t x = 42;
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(i, x);
        EXPECT_EQ(0u, queue->size()) << "i=" << i;
        EXPECT_EQ((i + 1) % capacity, queue->begin()) << "i=" << i;
    }
}

TEST_F(SubqueueTest, LoopThree) {
    for (size_t i = 0; i < 3*capacity; ++i) {
        EXPECT_TRUE(queue->add(1)) << "i=" << i;
        EXPECT_TRUE(queue->add(2)) << "i=" << i;
        EXPECT_TRUE(queue->add(3)) << "i=" << i;
        EXPECT_EQ(3u, queue->size()) << "i=" << i;
        EXPECT_EQ((3*i) % capacity, queue->begin()) << "i=" << i;
        entry_t x = 42;
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(1, x);
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(2, x);
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(3, x);
        EXPECT_FALSE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(0u, queue->size()) << "i=" << i;
        EXPECT_EQ((3*i + 3) % capacity, queue->begin()) << "i=" << i;
    }
}

TEST_F(SubqueueTest, LoopFull) {
    for (size_t i = 0; i < 3*capacity; ++i) {
        EXPECT_TRUE(queue->add(1)) << "i=" << i;
        EXPECT_TRUE(queue->add(2)) << "i=" << i;
        EXPECT_TRUE(queue->add(3)) << "i=" << i;
        EXPECT_TRUE(queue->add(4)) << "i=" << i;
        EXPECT_EQ(4u, queue->size()) << "i=" << i;
        EXPECT_EQ(0u, queue->begin()) << "i=" << i;
        entry_t x = 42;
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(1, x);
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(2, x);
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(3, x);
        EXPECT_TRUE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(4, x);
        EXPECT_FALSE(queue->get(x)) << "i=" << i;
        EXPECT_EQ(0u, queue->size()) << "i=" << i;
        EXPECT_EQ(0u, queue->begin()) << "i=" << i;
    }
}

TEST_F(SubqueueTest, OverFill) {
    EXPECT_TRUE(queue->add(1));
    EXPECT_TRUE(queue->add(2));
    EXPECT_TRUE(queue->add(3));
    EXPECT_TRUE(queue->add(4));
    EXPECT_FALSE(queue->add(5));
    entry_t x = 42;
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(1, x);
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(2, x);
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(3, x);
    EXPECT_TRUE(queue->get(x));
    EXPECT_EQ(4, x);
    EXPECT_FALSE(queue->get(x));
    EXPECT_EQ(0u, queue->size());
    EXPECT_EQ(0u, queue->begin());
}

TEST(SubqueuePtrTest, Construct) {
  subqueue_ptr<entry_t> p;
  EXPECT_EQ(nullptr, p.queue);
  EXPECT_EQ(64u, sizeof(p));
}

class QueueTest : public ::testing::Test {
    public:
    typedef concurrent_queue<entry_t> queue_t;
    enum { capacity = 4 };
    QueueTest() : queue(capacity) {}
    ~QueueTest() {}

    queue_t queue;
};

TEST_F(QueueTest, Construct) {
    EXPECT_EQ(0u, queue.size());
}

TEST_F(QueueTest, AddOne) {
    EXPECT_TRUE(queue.add(1));
    EXPECT_EQ(1u, queue.size());
}

TEST_F(QueueTest, GetOne) {
    EXPECT_TRUE(queue.add(1));
    entry_t x = 42;
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(1, x);
    EXPECT_EQ(0u, queue.size());
}

TEST_F(QueueTest, GetEmpty) {
    entry_t x = 42;
    EXPECT_FALSE(queue.get(x));
    EXPECT_EQ(42, x);
    EXPECT_EQ(0u, queue.size());
}

TEST_F(QueueTest, AddOneGetTwo) {
    EXPECT_TRUE(queue.add(1));
    entry_t x = 42;
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(1, x);
    x = 42;
    EXPECT_FALSE(queue.get(x));
    EXPECT_EQ(42, x);
    EXPECT_EQ(0u, queue.size());
}

TEST_F(QueueTest, AddGet) {
    EXPECT_TRUE(queue.add(1));
    EXPECT_TRUE(queue.add(2));
    entry_t x = 42;
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(1, x);
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(2, x);
    EXPECT_FALSE(queue.get(x));
    EXPECT_EQ(2, x);
    EXPECT_EQ(0u, queue.size());
}

class StdQueueTest : public ::testing::Test {
    public:
    typedef concurrent_std_queue<entry_t> queue_t;
    StdQueueTest() {}
    ~StdQueueTest() {}

    queue_t queue;
};

TEST_F(StdQueueTest, Construct) {
    EXPECT_EQ(0u, queue.size());
}

TEST_F(StdQueueTest, AddOne) {
    EXPECT_TRUE(queue.add(1));
    EXPECT_EQ(1u, queue.size());
}

TEST_F(StdQueueTest, GetOne) {
    EXPECT_TRUE(queue.add(1));
    entry_t x = 42;
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(1, x);
    EXPECT_EQ(0u, queue.size());
}

TEST_F(StdQueueTest, GetEmpty) {
    entry_t x = 42;
    EXPECT_FALSE(queue.get(x));
    EXPECT_EQ(42, x);
    EXPECT_EQ(0u, queue.size());
}

TEST_F(StdQueueTest, AddOneGetTwo) {
    EXPECT_TRUE(queue.add(1));
    entry_t x = 42;
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(1, x);
    x = 42;
    EXPECT_FALSE(queue.get(x));
    EXPECT_EQ(42, x);
    EXPECT_EQ(0u, queue.size());
}

TEST_F(StdQueueTest, AddGet) {
    EXPECT_TRUE(queue.add(1));
    EXPECT_TRUE(queue.add(2));
    entry_t x = 42;
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(1, x);
    EXPECT_TRUE(queue.get(x));
    EXPECT_EQ(2, x);
    EXPECT_FALSE(queue.get(x));
    EXPECT_EQ(2, x);
    EXPECT_EQ(0u, queue.size());
}

TEST_F(StdQueueTest, AddGetN) {
    const int N = 100;
    for (int i = 0; i < N; ++i) {
      queue.add(i);
    }
    EXPECT_EQ(size_t(N), queue.size());
    entry_t x = 42;
    for (int i = 0; i < N; ++i) {
      EXPECT_TRUE(queue.get(x));
      EXPECT_EQ(i, x);
    }
    EXPECT_EQ(0u, queue.size());
}

