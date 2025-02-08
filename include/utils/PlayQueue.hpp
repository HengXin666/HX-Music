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
#include <memory>
#include <variant>
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
struct Node {
    using List = std::list<Node<T>>;
    using Tp = typename std::list<Node<T>>::value_type;
    using iterator = typename List::iterator;

    std::unique_ptr<Node<T>> parent{nullptr};
    std::variant<T, List> data{nullptr};

    List& getList() {
        return std::get<List>(data);
    }

    iterator find(iterator it) {
        auto& list = getList();
        for (auto res = list.begin(); res != list.end(); ++res)
            if (*it == *res)
                return res;
        return list.end(); // 不可能
    }

    bool operator==(Node<T> const& that) const {
        if (parent != that.parent || data != that.data) {
            return false;
        }
        return true;
    }
};

template <typename T>
class NodeTree {
public:
    using Tp = Node<T>;
    using iterator = typename Tp::iterator;

    explicit NodeTree()
        : _root({nullptr, std::list<Tp>{}})
        , _it(_root.getList().begin())
        , _parentPtr(nullptr)
    {}

    template <typename U, 
        typename = std::enable_if_t<std::is_same_v<U, Tp>>>
    void insert(Tp* parent, U&& data) {
        if (!parent) {
            data.parent = nullptr;
            auto& rootList = _root.getList();
            rootList.emplace_back(std::forward<U>(data));
            if (_it == rootList.end()) {
                _it = rootList.begin();
            }
            return;
        }
        parent->getList().emplace_back(std::forward<U>(data));
    }

    iterator next() {
        auto& paList = _parentPtr ? _parentPtr->getList() : _root.getList();
        while (_it == paList.end()) {
            _it = ++_parentPtr->find(_it);
            _parentPtr = _parentPtr->parent.get();
        }
        ++_it;
        while (_it->data.index()) {
            _parentPtr = &*_it;
            _it = _it->getList().begin();
        }
        return _it;
    }

    iterator prev() {
        if (!_parentPtr && _it == _it->data.begin())
            return _it; // todo!!!
        --_it;
        while (_it == _it->data.begin()) {
            _it = _parentPtr;
            _parentPtr = _parentPtr->parent;
        }
        return _it;
    }

private:
    Tp _root;
    iterator _it;
    Tp* _parentPtr; // 只读
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