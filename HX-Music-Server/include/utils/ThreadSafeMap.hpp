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

#include <map>
#include <mutex>
#include <shared_mutex>

namespace HX::utils {

/**
 * @brief 线程安全红黑树
 * @tparam Key 
 * @tparam Val 
 */
template <typename Key, typename Val>
class ThreadSafeMap {
    std::map<Key, Val, std::less<>> _mp;
    mutable std::shared_mutex _mtx;
public:
    using iterator = typename decltype(_mp)::iterator;
private:
    struct CompareIterators {
        iterator it;    // 结果
        iterator end;   // 基准 map.end()
    };
public:
    ThreadSafeMap()
        : _mp{}
        , _mtx{}
    {}

    Val& at(Key const& key) {
        std::shared_lock _{_mtx};
        return _mp.at(key);
    }

    Val& at(Key const& key) const {
        std::shared_lock _{_mtx};
        return _mp.at(key);
    }

    template <typename K>
    CompareIterators find(K&& key) noexcept {
        std::shared_lock _{_mtx};
        return {_mp.find(std::forward<K>(key)), _mp.end()};
    }

    template <typename K>
    CompareIterators find(K&& key) const noexcept {
        std::shared_lock _{_mtx};
        return {_mp.find(std::forward<K>(key)), _mp.end()};
    }

    template <typename... Args>
    bool try_emplace(Args&&... args) {
        std::unique_lock _{_mtx};
        return _mp.try_emplace(std::forward<Args>(args)...).second;
    }

    ThreadSafeMap& operator=(ThreadSafeMap&&) noexcept = delete;
};

} // namespace HX::utils