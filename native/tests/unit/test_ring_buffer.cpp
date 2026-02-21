/**
 * @file test_ring_buffer.cpp
 * @brief Unit tests for SPSC ring buffer
 */

#include <gtest/gtest.h>
#include "spectra/core/ring_buffer.hpp"

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
