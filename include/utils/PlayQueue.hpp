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
#ifndef _HX_PLAY_QUEUE_H_
#define _HX_PLAY_QUEUE_H_

#include <QString>
#include <QVariant>
#include <QTreeWidgetItem>

namespace HX {

/**
 * @brief 播放队列
 */
class PlayQueue { // 需要重构
public:
    using Type = QString;
    using TypeOpt = typename std::optional<Type>;

    PlayQueue()
        : _pq{}
        , _it{_pq.end()}
    {}

    /**
     * @brief 歌单列表下一首
     * @return ItOpt 
     */
    TypeOpt next() {
        if (_it == --_pq.end()) {
            return {};
        }
        return *++_it;
    }

    /**
     * @brief 歌单列表上一首
     * @return ItOpt 
     */
    TypeOpt prev() {
        if (_it == _pq.begin()) {
            return {};
        }
        return *--_it;
    }

    /**
     * @brief 添加音乐进入队列尾部
     * @param it 
     */
    void push(Type const& uri) {
        _it = _pq.emplace(_pq.end(), uri);
        if (_pq.size() > 16)
            _pq.pop_front();
    }

    bool empty() const noexcept {
        return _pq.empty();
    }
private:
    std::list<Type> _pq; // 歌曲路径
    decltype(_pq.begin()) _it; // 当前歌曲迭代器
};

} // namespace HX

#endif // !_HX_PLAY_QUEUE_H_