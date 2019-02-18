#include <atomic_queue.h>

#include <limits.h>
#include <stdlib.h>

#include <gtest/gtest.h>

class MallocScopedPtr {
    public:
    MallocScopedPtr(void* p) : p_(p) {}
    ~MallocScopedPtr() { ::free(p_); }
    operator void*() const { return p_; }
    private:
    void* p_;
};

class QueueTest : public ::testing::Test {
    public:
    typedef int Entry;
    enum { MAX_LEN = 100 };
    QueueTest() : memory_(malloc(MAX_LEN*sizeof(Entry))), queue_(memory_, MAX_LEN*sizeof(Entry)) {}

    MallocScopedPtr memory_;
    atomic_queue<Entry> queue_;
};

TEST_F(QueueTest, Construct) {
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(0u, queue_.Length());
    EXPECT_EQ(QueueTest::MAX_LEN, queue_.Capacity());
}

TEST_F(QueueTest, EnqueueOne) {
    EXPECT_TRUE(queue_.Enqueue(1));
    queue_.EndEnqueue();
    EXPECT_FALSE(queue_.Empty());
    EXPECT_EQ(1u, queue_.Length());
}

TEST_F(QueueTest, FastEnqueueOne) {
    EXPECT_TRUE(queue_.FastEnqueue(1));
    queue_.EndEnqueue();
    EXPECT_FALSE(queue_.Empty());
    EXPECT_EQ(1u, queue_.Length());
}

TEST_F(QueueTest, DequeueOne) {
    EXPECT_TRUE(queue_.Enqueue(1));
    queue_.EndEnqueue();
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(0u, queue_.Length());
}

TEST_F(QueueTest, FastEnqueueDequeueOne) {
    EXPECT_TRUE(queue_.FastEnqueue(1));
    queue_.EndEnqueue();
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(0u, queue_.Length());
}

TEST_F(QueueTest, DequeueEmpty) {
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, EnqueueDequeue) {
    EXPECT_TRUE(queue_.Enqueue(1));
    EXPECT_TRUE(queue_.Enqueue(2));
    EXPECT_TRUE(queue_.Enqueue(3));
    queue_.EndEnqueue();
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(2, queue_.Dequeue());
    EXPECT_EQ(3, queue_.Dequeue());
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, FastEnqueueDequeue) {
    EXPECT_TRUE(queue_.FastEnqueue(1));
    EXPECT_TRUE(queue_.FastEnqueue(2));
    EXPECT_TRUE(queue_.FastEnqueue(3));
    queue_.EndEnqueue();
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(2, queue_.Dequeue());
    EXPECT_EQ(3, queue_.Dequeue());
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, EnqueueDequeueTwice) {
    EXPECT_TRUE(queue_.Enqueue(1));
    EXPECT_TRUE(queue_.Enqueue(2));
    EXPECT_TRUE(queue_.Enqueue(3));
    EXPECT_TRUE(queue_.Enqueue(4));
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(2, queue_.Dequeue());
    EXPECT_TRUE(queue_.Enqueue(5));
    EXPECT_TRUE(queue_.Enqueue(6));
    queue_.EndEnqueue();
    EXPECT_EQ(3, queue_.Dequeue());
    EXPECT_EQ(4, queue_.Dequeue());
    EXPECT_EQ(5, queue_.Dequeue());
    EXPECT_EQ(6, queue_.Dequeue());
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, FastEnqueueDequeueTwice) {
    EXPECT_TRUE(queue_.FastEnqueue(1));
    EXPECT_TRUE(queue_.FastEnqueue(2));
    EXPECT_TRUE(queue_.FastEnqueue(3));
    EXPECT_TRUE(queue_.FastEnqueue(4));
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(2, queue_.Dequeue());
    EXPECT_TRUE(queue_.FastEnqueue(5));
    EXPECT_TRUE(queue_.FastEnqueue(6));
    queue_.EndEnqueue();
    EXPECT_EQ(3, queue_.Dequeue());
    EXPECT_EQ(4, queue_.Dequeue());
    EXPECT_EQ(5, queue_.Dequeue());
    EXPECT_EQ(6, queue_.Dequeue());
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, FastEnqueueThenEnqueue) {
    EXPECT_TRUE(queue_.FastEnqueue(1));
    EXPECT_TRUE(queue_.FastEnqueue(2));
    EXPECT_TRUE(queue_.Enqueue(3));
    EXPECT_TRUE(queue_.Enqueue(4));
    queue_.EndEnqueue();
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(2, queue_.Dequeue());
    EXPECT_EQ(3, queue_.Dequeue());
    EXPECT_EQ(4, queue_.Dequeue());
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, Reset) {
    EXPECT_TRUE(queue_.Enqueue(1));
    EXPECT_TRUE(queue_.Enqueue(2));
    EXPECT_TRUE(queue_.Enqueue(3));
    queue_.EndEnqueue();
    EXPECT_FALSE(queue_.Empty());
    EXPECT_EQ(3u, queue_.Length());
    queue_.Reset();
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(0u, queue_.Length());
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, EnqueueEmptyValue) {
    EXPECT_FALSE(queue_.Enqueue(0));
    queue_.EndEnqueue();
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(0u, queue_.Length());
}

TEST_F(QueueTest, FastEnqueueEmptyValue) {
    EXPECT_FALSE(queue_.FastEnqueue(0));
    queue_.EndEnqueue();
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(0u, queue_.Length());
}

TEST_F(QueueTest, EnqueueRange) {
    Entry e[] = { 1, 2, 3 };
    EXPECT_TRUE(queue_.EnqueueRange(3, e));
    queue_.EndEnqueue();
    EXPECT_FALSE(queue_.Empty());
    EXPECT_EQ(3u, queue_.Length());
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(2, queue_.Dequeue());
    EXPECT_EQ(3, queue_.Dequeue());
    EXPECT_EQ(0, queue_.Dequeue());
}

TEST_F(QueueTest, FastEnqueueThenEnqueueRange) {
    EXPECT_TRUE(queue_.FastEnqueue(1));
    EXPECT_TRUE(queue_.FastEnqueue(2));
    Entry e[] = { 3, 4 };
    EXPECT_TRUE(queue_.EnqueueRange(2, e));
    queue_.EndEnqueue();
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(2, queue_.Dequeue());
    EXPECT_EQ(3, queue_.Dequeue());
    EXPECT_EQ(4, queue_.Dequeue());
    EXPECT_EQ(0, queue_.Dequeue());
}

// Test custom empty value.
class QueueTest1 : public ::testing::Test {
    public:
    typedef int Entry;
    enum { MAX_LEN = 100 };
    QueueTest1() : memory_(malloc(MAX_LEN*sizeof(Entry))), queue_(memory_, MAX_LEN*sizeof(Entry), INT_MAX) {}

    MallocScopedPtr memory_;
    atomic_queue<Entry> queue_;
};

TEST_F(QueueTest1, EmptyValue) {
    EXPECT_TRUE(queue_.FastEnqueue(1));
    EXPECT_EQ(INT_MAX, queue_.EmptyValue());
    queue_.EndEnqueue();
    EXPECT_FALSE(queue_.Empty());
    EXPECT_EQ(1, queue_.Dequeue());
    EXPECT_EQ(INT_MAX, queue_.Dequeue());
    EXPECT_EQ(INT_MAX, queue_.Dequeue());
    EXPECT_EQ(INT_MAX, queue_.Dequeue());
}

