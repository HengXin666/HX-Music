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

#include <random>

#include <QString>
#include <QVariant>

#include <utils/FileTree.hpp>

namespace HX {

class PlayQueue : public FileTree<QString> {
public:
    using ItOpt = typename std::optional<FileTree<QString>::iterator>;
    using Node = typename FileTree<QString>::Tp;

    explicit PlayQueue()
        : FileTree<QString>()
        , _pq()
        , _pqIt(_pq.end())
    {}

    /**
     * @brief 上一首音乐
     * @return ItOpt 
     */
    ItOpt prev() {
        if (_pq.empty())
            return {};
        if (_pq.size() == 1)
            return *(_pqIt = _pq.begin());
        if (_pqIt == _pq.begin())
            return *_pqIt;
        return _pqIt == _pq.end() ? *----_pqIt : *--_pqIt;
    }

    ItOpt next() {
        if (_root.getList().empty() || !_root.begin())
            return {};
        if (_pqIt == _pq.end() || ++_pqIt == _pq.end()) {
            auto res = FileTree<QString>::next();
            push(*res);
            return res;
        }
        return *_pqIt;
    }

    /**
     * @brief 随机下一首音乐
     * @return ItOpt 
     */
    ItOpt randomNext() {
        if (_root.getList().empty() || !_root.begin())
            return {};
        if (_pqIt == _pq.end() || ++_pqIt == _pq.end())
            return random();
        return *_pqIt;
    }

    /**
     * @brief 添加音乐进入队列尾部
     * @param it 
     */
    void push(iterator it) {
        _pq.emplace_back(it);
        _pqIt = _pq.end();
        if (_pq.size() >= 16) {
            _pq.pop_front();
        }
    }

private:
    ItOpt random() {
        std::mt19937 rng{std::random_device{}()};
        ItOpt it = {};
        std::size_t nextCnt = std::uniform_int_distribution<std::size_t>{1, _cnt}(rng);
        for (std::size_t i = 0; i < nextCnt; ++i) {
            it = FileTree<QString>::next();
        }
        push(*it);
        return it;
    }

    // 只和`random`相关
    std::list<iterator> _pq;
    std::list<iterator>::iterator _pqIt;
};

} // namespace HX

Q_DECLARE_METATYPE(HX::PlayQueue::iterator);

#endif // !_HX_PLAY_QUEUE_H_