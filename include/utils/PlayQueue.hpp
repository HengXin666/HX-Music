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

#include <list>
#include <variant>
#include <optional>
#include <type_traits>
#include <QString>

/*
需求:
支持上一、下一
支持在文件夹中穿梭
支持快速的增删
*/

namespace HX {

template <typename T>
class NodeTree;

template <typename T>
class Node {
    friend class NodeTree<T>;
public:
    using Tp = Node<T>;
    using List = std::list<Tp>;
    using LT = typename std::list<Tp>::value_type;
    using iterator = typename List::iterator;

    T& getData() {
        return std::get<T>(_data);
    }

    explicit Node(T&& t)
        : _parentIt()
        , _data(std::forward<T>(t))
    {}

    template <typename U, 
        typename = std::enable_if_t<
            std::is_same_v<T, U> || std::is_same_v<List, U>>>
    explicit Node(std::optional<iterator> pIt, U&& u)
        : _parentIt(pIt)
        , _data(std::forward<U>(u))
    {}

private:
    std::optional<iterator> _parentIt;
    std::variant<T, List> _data;

    bool isList() const {
        return _data.index();
    }

    List& getList() {
        return std::get<List>(_data);
    }

    /**
     * @brief 返回第一个文件的迭代器, 如果有文件夹会进入
     * @return iterator 
     */
    std::optional<iterator> begin() {
        auto& list = getList();
        auto res = list.begin();
        // 如果是多个空文件夹, 需要考虑略过
        for (; res != list.end(); ++res) {
            if (res->isList()) {
                if (auto ans = res->begin()) {
                    return *ans;
                }
            } else {
                return res;
            }
        }
        return {};
    }

    /**
     * @brief 返回最后一个文件的迭代器, 如果有文件夹会进入
     * @return iterator 
     */
    std::optional<iterator> end() {
        auto& list = getList();
        auto res = list.end();
        // 如果是多个空文件夹, 需要考虑略过
        for (; res != list.begin(); ) {
            --res;
            if (res->isList()) {
                if (auto ans = res->end()) {
                    return *ans;
                }
            } else {
                return res;
            }
        }
        return {};
    }

    bool operator==(Node<T> const& that) const {
        return _parentIt == that._parentIt 
            && _data == that._data;
    }
};

template <typename T>
class NodeTree {
public:
    using Tp = Node<T>;
    using List = typename Tp::List;
    using iterator = typename Tp::iterator;

    explicit NodeTree()
        : _root({}, List{})
        , _it(_root.getList().end())
    {}

    template <typename U, 
        typename = std::enable_if_t<std::is_same_v<U, Tp>>>
    iterator insert(std::optional<iterator> parentIt, U&& data) {
        auto& list = parentIt ? (*parentIt)->getList() : _root.getList();
        data._parentIt = parentIt;
        list.emplace_back(std::forward<U>(data));
        auto res = --list.end();
        // 如果播放队列没有歌曲, 那么更新一下; 前提是: 添加的不是文件夹
        if (_it == _root.getList().end() && !res->isList()) {
            _it = *_root.begin();
        }
        return res;
    }

    bool empty() const noexcept {
        return _root.getList().size();
    }

    std::optional<iterator> next() {
        // 保证有文件, 而不是全是文件夹
        if (auto list = _root.getList(); _it == _root.getList().end()) [[unlikely]] {
            if (list.empty()) {
                return {};
            }
            return _it = *_root.begin();
        }

        // 下一个
        auto mae = _it++;
        
        // 不是尾部; 如果是, 则迭代退出
        while (mae->_parentIt && _it == (*mae->_parentIt)->getList().end()) {
            _it = *mae->_parentIt;  // 退出
            mae = _it++;            // 下一个
        }

        // 是根, 并且是尾部
        if (!mae->_parentIt && _it == _root.getList().end()) {
            return _it = *_root.begin(); // 循环一次
        }

        // 新节点, 看看需不需要进入文件夹
        if (_it->isList()) {
            if (auto ans = _it->begin()) {
                return _it = *ans;
            }
            // 如果是空文件夹, 就继续
            return next();
        }
        return _it;
    }

    std::optional<iterator> prev() {
        // 保证有文件, 而不是全是文件夹
        if (auto list = _root.getList(); _it == _root.getList().end()) [[unlikely]] {
            if (list.empty()) {
                return {};
            }
            return _it = *_root.begin();
        }

        // 不是头部; 如果是, 则迭代退出
        while (_it->_parentIt && _it == (*_it->_parentIt)->getList().begin()) {
            _it = *_it->_parentIt;  // 退出
        }

        // 是根, 并且是头部
        if (!_it->_parentIt && _it == _root.getList().begin()) {
            return _it = *_root.end();
        }

        // 上一个
        --_it;
        
        // 新节点, 看看需不需要进入文件夹
        if (_it->isList()) {
            if (auto ans = _it->end()) {
                return _it = *ans;
            }
            // 如果是空文件夹, 就继续
            return prev();
        }
        return _it;
    }

private:
    Tp _root;
    iterator _it;
};

class PlayQueue : public NodeTree<QString> {
public:
    explicit PlayQueue()
        : NodeTree<QString>()
    {}
};

} // namespace HX

// struct PlayQueueNode {
//     using iterator = std::list<PlayQueueNode>::iterator;
//     using const_iterator = std::list<PlayQueueNode>::const_iterator;

//     /// @brief 路径
//     QString _path;

//     /// @brief 子节点
//     std::list<PlayQueueNode> _child;

//     /// @brief 父节点
//     iterator _parent;
// };

// namespace internal {

// inline static std::list<PlayQueueNode> PlayQueueRootNodeEnd{};

// inline static PlayQueueNode::iterator PlayQueueRootNodeEndIt 
//     = PlayQueueRootNodeEnd.end();

// } // namespace internal

// /**
//  * @brief 播放队列
//  */
// class PlayQueue {
//     // 支持随机访问、支持清除
//     // 如果是給别人保存迭代器那 下一个就是++it
//     // 可随机跳呢?
// public:
//     using iterator = PlayQueueNode::iterator;
//     using const_iterator = PlayQueueNode::const_iterator;

//     explicit PlayQueue()
//         : _root({
//             "",
//             {},
//             internal::PlayQueueRootNodeEndIt})
//         , _it(_root._child.begin())
//     {}

//     iterator insertBack(iterator it, PlayQueueNode const& data) {
//         return it->_child.insert(it->_child.end(), data);
//     }

//     iterator next() {
//         if (_it == _it->_parent->_child.end())
//     }

//     iterator prev() {

//     }

// private:
//     PlayQueueNode _root;
//     iterator _it; // 当前
// };

#endif // !_HX_PLAY_QUEUE_H_