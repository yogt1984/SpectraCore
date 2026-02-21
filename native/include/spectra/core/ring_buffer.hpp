/**
 * @file ring_buffer.hpp
 * @brief Lock-free Single-Producer Single-Consumer ring buffer
 */

#ifndef SPECTRA_CORE_RING_BUFFER_HPP
#define SPECTRA_CORE_RING_BUFFER_HPP

#include <atomic>
#include <cstddef>
#include <array>
#include <type_traits>

namespace spectra {

/**
 * @brief Lock-free SPSC ring buffer for real-time audio streaming
 *
 * This implementation is wait-free for both producer and consumer when used
 * correctly (single producer thread, single consumer thread).
 *
 * @tparam T Element type (must be trivially copyable)
 * @tparam Size Buffer capacity (must be power of two)
 */
template<typename T, size_t Size>
class SpscRingBuffer {
    static_assert(std::is_trivially_copyable_v<T>,
                  "SpscRingBuffer requires trivially copyable types");
    static_assert((Size & (Size - 1)) == 0,
                  "SpscRingBuffer size must be a power of two");

public:
    SpscRingBuffer() : head_(0), tail_(0) {}

    // Non-copyable, non-movable
    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;
    SpscRingBuffer(SpscRingBuffer&&) = delete;
    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    /**
     * @brief Push a single element (producer thread only)
     * @param sample Element to push
     * @return true if successful, false if buffer is full
     */
    bool push(const T& sample) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t next_head = (current_head + 1) & mask_;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }

        buffer_[current_head] = sample;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    /**
     * @brief Push multiple elements (producer thread only)
     * @param samples Pointer to elements
     * @param count Number of elements to push
     * @return Number of elements actually pushed
     */
    size_t push_batch(const T* samples, size_t count) {
        size_t pushed = 0;
        for (size_t i = 0; i < count; ++i) {
            if (!push(samples[i])) {
                break;
            }
            ++pushed;
        }
        return pushed;
    }

    /**
     * @brief Pop a single element (consumer thread only)
     * @param sample Output element
     * @return true if successful, false if buffer is empty
     */
    bool pop(T& sample) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);

        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }

        sample = buffer_[current_tail];
        tail_.store((current_tail + 1) & mask_, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pop multiple elements (consumer thread only)
     * @param samples Output buffer
     * @param max_count Maximum number of elements to pop
     * @return Number of elements actually popped
     */
    size_t pop_batch(T* samples, size_t max_count) {
        size_t popped = 0;
        for (size_t i = 0; i < max_count; ++i) {
            if (!pop(samples[i])) {
                break;
            }
            ++popped;
        }
        return popped;
    }

    /**
     * @brief Get number of elements available for reading
     * @return Number of elements in buffer
     */
    [[nodiscard]] size_t available() const {
        const size_t head = head_.load(std::memory_order_acquire);
        const size_t tail = tail_.load(std::memory_order_acquire);
        return (head - tail) & mask_;
    }

    /**
     * @brief Get number of free slots for writing
     * @return Number of free slots
     */
    [[nodiscard]] size_t free_slots() const {
        return Size - 1 - available();
    }

    /**
     * @brief Check if buffer is empty
     */
    [[nodiscard]] bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    /**
     * @brief Check if buffer is full
     */
    [[nodiscard]] bool full() const {
        const size_t next_head = (head_.load(std::memory_order_acquire) + 1) & mask_;
        return next_head == tail_.load(std::memory_order_acquire);
    }

    /**
     * @brief Reset buffer to empty state
     * @warning Only call when no concurrent access is happening
     */
    void reset() {
        head_.store(0, std::memory_order_release);
        tail_.store(0, std::memory_order_release);
    }

    /**
     * @brief Get buffer capacity
     */
    static constexpr size_t capacity() { return Size - 1; }

private:
    static constexpr size_t mask_ = Size - 1;

    // Cache line alignment to prevent false sharing
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
    alignas(64) std::array<T, Size> buffer_;
};

// Common buffer sizes
using AudioRingBuffer1K = SpscRingBuffer<float, 1024>;
using AudioRingBuffer4K = SpscRingBuffer<float, 4096>;
using AudioRingBuffer16K = SpscRingBuffer<float, 16384>;

} // namespace spectra

#endif // SPECTRA_CORE_RING_BUFFER_HPP
