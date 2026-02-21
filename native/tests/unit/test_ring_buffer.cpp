/**
 * @file test_ring_buffer.cpp
 * @brief Unit tests for SPSC ring buffer
 */

#include <gtest/gtest.h>
#include "spectra/core/ring_buffer.hpp"
#include <thread>
#include <vector>
#include <numeric>
#include <atomic>

using namespace spectra;

class RingBufferTest : public ::testing::Test {
protected:
    SpscRingBuffer<float, 16> buffer;
};

TEST_F(RingBufferTest, InitiallyEmpty) {
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    EXPECT_EQ(buffer.available(), 0u);
}

TEST_F(RingBufferTest, PushAndPop) {
    float sample = 1.5f;
    EXPECT_TRUE(buffer.push(sample));
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.available(), 1u);

    float popped;
    EXPECT_TRUE(buffer.pop(popped));
    EXPECT_FLOAT_EQ(popped, sample);
    EXPECT_TRUE(buffer.empty());
}

TEST_F(RingBufferTest, FillToCapacity) {
    // Capacity is Size - 1 for SPSC
    for (size_t i = 0; i < buffer.capacity(); ++i) {
        EXPECT_TRUE(buffer.push(static_cast<float>(i)));
    }
    EXPECT_TRUE(buffer.full());
    EXPECT_FALSE(buffer.push(999.0f));
}

TEST_F(RingBufferTest, FIFO_Order) {
    for (int i = 0; i < 5; ++i) {
        buffer.push(static_cast<float>(i));
    }

    for (int i = 0; i < 5; ++i) {
        float val;
        EXPECT_TRUE(buffer.pop(val));
        EXPECT_FLOAT_EQ(val, static_cast<float>(i));
    }
}

TEST_F(RingBufferTest, BatchOperations) {
    float input[] = {1.0f, 2.0f, 3.0f, 4.0f};
    EXPECT_EQ(buffer.push_batch(input, 4), 4u);

    float output[4];
    EXPECT_EQ(buffer.pop_batch(output, 4), 4u);

    for (int i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(output[i], input[i]);
    }
}

TEST_F(RingBufferTest, Reset) {
    buffer.push(1.0f);
    buffer.push(2.0f);
    EXPECT_FALSE(buffer.empty());

    buffer.reset();
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.available(), 0u);
}

TEST_F(RingBufferTest, WrapAround) {
    // Fill and empty multiple times to test wrap-around
    for (int round = 0; round < 5; ++round) {
        for (size_t i = 0; i < 10; ++i) {
            buffer.push(static_cast<float>(round * 10 + i));
        }
        for (size_t i = 0; i < 10; ++i) {
            float val;
            EXPECT_TRUE(buffer.pop(val));
            EXPECT_FLOAT_EQ(val, static_cast<float>(round * 10 + i));
        }
    }
}

// ============================================================================
// CORE-004: Additional rigorous ring buffer tests
// ============================================================================

TEST_F(RingBufferTest, CapacityIsSizeMinusOne) {
    // SPSC ring buffer uses one slot to distinguish full from empty
    // Capacity should be Size - 1
    SpscRingBuffer<float, 16> buf16;
    EXPECT_EQ(buf16.capacity(), 15u);

    SpscRingBuffer<float, 64> buf64;
    EXPECT_EQ(buf64.capacity(), 63u);

    SpscRingBuffer<float, 1024> buf1k;
    EXPECT_EQ(buf1k.capacity(), 1023u);
}

TEST_F(RingBufferTest, PopFromEmpty) {
    float val = 999.0f;
    EXPECT_FALSE(buffer.pop(val));
    // Value should be unchanged on failed pop
    EXPECT_FLOAT_EQ(val, 999.0f);
}

TEST_F(RingBufferTest, PushToFull) {
    // Fill buffer
    for (size_t i = 0; i < buffer.capacity(); ++i) {
        EXPECT_TRUE(buffer.push(static_cast<float>(i)));
    }
    EXPECT_TRUE(buffer.full());

    // Push should fail
    EXPECT_FALSE(buffer.push(999.0f));
    EXPECT_TRUE(buffer.full());

    // Available should equal capacity
    EXPECT_EQ(buffer.available(), buffer.capacity());
}

TEST_F(RingBufferTest, FreeSlots) {
    EXPECT_EQ(buffer.free_slots(), buffer.capacity());

    buffer.push(1.0f);
    EXPECT_EQ(buffer.free_slots(), buffer.capacity() - 1);

    buffer.push(2.0f);
    EXPECT_EQ(buffer.free_slots(), buffer.capacity() - 2);

    float val;
    buffer.pop(val);
    EXPECT_EQ(buffer.free_slots(), buffer.capacity() - 1);
}

TEST_F(RingBufferTest, BatchPushPartial) {
    // Fill most of the buffer
    for (size_t i = 0; i < buffer.capacity() - 2; ++i) {
        buffer.push(static_cast<float>(i));
    }

    // Try to push more than available space
    float input[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    size_t pushed = buffer.push_batch(input, 5);
    EXPECT_EQ(pushed, 2u);  // Only 2 slots available
}

TEST_F(RingBufferTest, BatchPopPartial) {
    buffer.push(1.0f);
    buffer.push(2.0f);
    buffer.push(3.0f);

    float output[10];
    size_t popped = buffer.pop_batch(output, 10);
    EXPECT_EQ(popped, 3u);  // Only 3 elements available
    EXPECT_FLOAT_EQ(output[0], 1.0f);
    EXPECT_FLOAT_EQ(output[1], 2.0f);
    EXPECT_FLOAT_EQ(output[2], 3.0f);
}

TEST_F(RingBufferTest, InterleavedPushPop) {
    // Simulate streaming: push some, pop some, repeat
    for (int round = 0; round < 100; ++round) {
        // Push 3 elements
        for (int i = 0; i < 3; ++i) {
            buffer.push(static_cast<float>(round * 3 + i));
        }

        // Pop 3 elements
        for (int i = 0; i < 3; ++i) {
            float val;
            EXPECT_TRUE(buffer.pop(val));
            EXPECT_FLOAT_EQ(val, static_cast<float>(round * 3 + i));
        }
    }
    EXPECT_TRUE(buffer.empty());
}

TEST_F(RingBufferTest, LargerBufferSize) {
    SpscRingBuffer<float, 4096> large_buffer;

    // Fill completely
    for (size_t i = 0; i < large_buffer.capacity(); ++i) {
        EXPECT_TRUE(large_buffer.push(static_cast<float>(i)));
    }
    EXPECT_TRUE(large_buffer.full());
    EXPECT_EQ(large_buffer.available(), large_buffer.capacity());

    // Verify all data
    for (size_t i = 0; i < large_buffer.capacity(); ++i) {
        float val;
        EXPECT_TRUE(large_buffer.pop(val));
        EXPECT_FLOAT_EQ(val, static_cast<float>(i));
    }
    EXPECT_TRUE(large_buffer.empty());
}

TEST_F(RingBufferTest, DataIntegrity) {
    // Use a pattern that's easy to verify
    const size_t iterations = 1000;

    for (size_t iter = 0; iter < iterations; ++iter) {
        float expected = static_cast<float>(iter) * 1.5f + 0.25f;
        buffer.push(expected);

        float actual;
        buffer.pop(actual);
        EXPECT_FLOAT_EQ(actual, expected) << "Iteration: " << iter;
    }
}

// Multi-threaded stress test
TEST(RingBufferConcurrentTest, ProducerConsumer) {
    SpscRingBuffer<int, 1024> buffer;
    const int num_items = 100000;
    std::atomic<bool> producer_done{false};
    std::atomic<long long> consumer_sum{0};

    // Producer thread
    std::thread producer([&]() {
        for (int i = 1; i <= num_items; ++i) {
            while (!buffer.push(i)) {
                // Busy wait (not ideal but simple for testing)
                std::this_thread::yield();
            }
        }
        producer_done.store(true, std::memory_order_release);
    });

    // Consumer thread
    std::thread consumer([&]() {
        long long sum = 0;
        int received = 0;
        while (received < num_items) {
            int val;
            if (buffer.pop(val)) {
                sum += val;
                ++received;
            } else if (producer_done.load(std::memory_order_acquire) && buffer.empty()) {
                break;
            } else {
                std::this_thread::yield();
            }
        }
        consumer_sum.store(sum, std::memory_order_release);
    });

    producer.join();
    consumer.join();

    // Sum of 1 to n = n*(n+1)/2
    long long expected_sum = static_cast<long long>(num_items) * (num_items + 1) / 2;
    EXPECT_EQ(consumer_sum.load(), expected_sum);
}

TEST(RingBufferConcurrentTest, FIFOOrderPreserved) {
    SpscRingBuffer<int, 256> buffer;
    const int num_items = 50000;
    std::atomic<bool> order_valid{true};
    std::atomic<bool> producer_done{false};

    std::thread producer([&]() {
        for (int i = 0; i < num_items; ++i) {
            while (!buffer.push(i)) {
                std::this_thread::yield();
            }
        }
        producer_done.store(true, std::memory_order_release);
    });

    std::thread consumer([&]() {
        int expected = 0;
        while (expected < num_items) {
            int val;
            if (buffer.pop(val)) {
                if (val != expected) {
                    order_valid.store(false, std::memory_order_release);
                    break;
                }
                ++expected;
            } else if (producer_done.load(std::memory_order_acquire) && buffer.empty()) {
                break;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    EXPECT_TRUE(order_valid.load()) << "FIFO order was violated";
}

TEST(RingBufferConcurrentTest, NoDataLoss) {
    SpscRingBuffer<int, 128> buffer;
    const int num_items = 10000;
    std::vector<bool> received(num_items, false);
    std::atomic<bool> producer_done{false};

    std::thread producer([&]() {
        for (int i = 0; i < num_items; ++i) {
            while (!buffer.push(i)) {
                std::this_thread::yield();
            }
        }
        producer_done.store(true, std::memory_order_release);
    });

    std::thread consumer([&]() {
        int count = 0;
        while (count < num_items) {
            int val;
            if (buffer.pop(val)) {
                if (val >= 0 && val < num_items) {
                    received[val] = true;
                }
                ++count;
            } else if (producer_done.load(std::memory_order_acquire) && buffer.empty()) {
                break;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    // Verify all items were received
    for (int i = 0; i < num_items; ++i) {
        EXPECT_TRUE(received[i]) << "Item " << i << " was lost";
    }
}

TEST(RingBufferConcurrentTest, HighThroughput) {
    SpscRingBuffer<float, 4096> buffer;
    const size_t num_items = 1000000;
    std::atomic<size_t> items_consumed{0};

    std::thread producer([&]() {
        for (size_t i = 0; i < num_items; ++i) {
            while (!buffer.push(static_cast<float>(i))) {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&]() {
        size_t count = 0;
        while (count < num_items) {
            float val;
            if (buffer.pop(val)) {
                ++count;
            } else {
                std::this_thread::yield();
            }
        }
        items_consumed.store(count, std::memory_order_release);
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(items_consumed.load(), num_items);
}
