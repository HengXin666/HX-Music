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

#include <QString>
#include <QVariant>
#include <QTreeWidgetItem>

namespace HX {

/**
 * @brief 播放队列, 支持安全的前后切换和队列维护
 */
class PlayQueue {
public:
    using Type = uint64_t;
    using TypeOpt = std::optional<Type>;
    using Iterator = std::list<Type>::iterator;

    PlayQueue() = default;

    /**
     * @brief 播放下一首
     * @return 下一首歌曲ID, 如果没有则返回空
     */
    TypeOpt next() {
        if (_pq.empty() || !_it.has_value()) {
            return {};
        }
        auto cur = *_it;
        auto last = std::prev(_pq.end());
        if (cur == last) {
            return {}; // 已经是最后一首
        }
        ++cur;
        _it = cur;
        return **_it;
    }

    /**
     * @brief 播放上一首
     * @return 上一首歌曲ID, 如果没有则返回空
     */
    TypeOpt prev() {
        if (_pq.empty() || !_it.has_value()) {
            return {};
        }
        auto cur = *_it;
        if (cur == _pq.begin()) {
            return {}; // 已经是第一首
        }
        --cur;
        _it = cur;
        return **_it;
    }

    /**
     * @brief 添加音乐到队列尾部, 队列最多保留16首
     * @param uri 歌曲ID
     */
    void push(Type const& uri) {
        _pq.emplace_back(uri);
        if (!_it.has_value()) {
            _it = std::prev(_pq.end()); // 第一次添加时初始化_it
        } else {
            // 插入在末尾时, 如果_it无效则更新到新元素
            _it = std::prev(_pq.end());
        }

        // 队列溢出时删除最前的一个
        if (_pq.size() > 16) {
            auto frontIt = _pq.begin();
            if (_it.value() == frontIt) {
                _pq.pop_front();
                _it = _pq.begin();
            } else {
                _pq.pop_front();
            }
        }
    }

    bool empty() const noexcept {
        return _pq.empty();
    }

private:
    std::list<Type> _pq;              // 歌曲路径队列
    std::optional<Iterator> _it;      // 当前歌曲迭代器, 用optional保证有效性
};

} // namespace HX

