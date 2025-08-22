#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HX-Music.
 *
 * HX-Music is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HX-Music is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HX-Music.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _HX_SPSC_QUEUE_H_
#define _HX_SPSC_QUEUE_H_

#include <atomic>
#include <vector>
#include <optional>
#include <cassert>

namespace HX {

// 简单的环形 SPSC 队列(容量必须是 2 的幂)
template <typename T>
class SpscQueue {
public:
    explicit SpscQueue(size_t capacity)
        : _capacity{capacity}
        , _mask{capacity - 1}
        , _buffer{capacity}
        , _head{}
        , _tail{}
    {
        assert((capacity & _mask) == 0); // 2^n
        _head.store(0, std::memory_order_relaxed);
        _tail.store(0, std::memory_order_relaxed);
    }

    // 生产者: 尝试 push, 有空间返回 true
    bool tryPush(T&& item) {
        size_t tail = _tail.load(std::memory_order_relaxed);
        size_t next = (tail + 1) & _mask;
        if (next == _head.load(std::memory_order_acquire)) [[unlikely]] {
            return false; // 满了
        }
        _buffer[tail] = std::move(item);
        _tail.store(next, std::memory_order_release);
        return true;
    }

    // 消费者: 尝试 pop, 成功返回 optional
    std::optional<T> tryPop() {
        size_t head = _head.load(std::memory_order_relaxed);
        if (head == _tail.load(std::memory_order_acquire)) [[unlikely]] {
            return std::nullopt; // 空
        }
        T item = std::move(_buffer[head]);
        _head.store((head + 1) & _mask, std::memory_order_release);
        return item;
    }

private:
    const size_t _capacity;
    const size_t _mask;
    std::vector<T> _buffer;
    std::atomic<size_t> _head;
    std::atomic<size_t> _tail;
};

} // namespace HX

#endif // !_HX_SPSC_QUEUE_H_