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
#ifndef _HX_FILE_TREE_H_
#define _HX_FILE_TREE_H_

#include <list>
#include <variant>
#include <optional>
#include <type_traits>

namespace HX {

template <typename T>
class FileTree;

template <typename T>
class FileTreeNode {
    friend class FileTree<T>;
public:
    using Tp = FileTreeNode<T>;
    using List = std::list<Tp>;
    using LT = typename std::list<Tp>::value_type;
    using iterator = typename List::iterator;

    template <typename U, 
        typename = std::enable_if_t<
            std::is_same_v<T, U> || std::is_same_v<List, U>>>
    explicit FileTreeNode(U&& u)
        : _parentIt()
        , _data(std::forward<U>(u))
    {}

    template <typename U, 
        typename = std::enable_if_t<
            std::is_same_v<T, U> || std::is_same_v<List, U>>>
    explicit FileTreeNode(std::optional<iterator> pIt, U&& u)
        : _parentIt(pIt)
        , _data(std::forward<U>(u))
    {}

    T& getData() {
        return std::get<T>(_data);
    }

    bool isList() const {
        return _data.index();
    }

    /**
     * @brief 获取list
     * @warning 请保证`_data`处于list类型, 否则抛出异常
     * @return List& 
     */
    List& getList() {
        return std::get<List>(_data);
    }

    /**
     * @brief 返回第一个文件的迭代器, 如果有文件夹会进入
     * @return std::optional<iterator> 
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
     * @return std::optional<iterator> 
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

    bool operator==(FileTreeNode<T> const& that) const {
        return _parentIt == that._parentIt 
            && _data == that._data;
    }

private:
    std::optional<iterator> _parentIt;
    std::variant<T, List> _data;
    std::size_t _cnt = 0;
};

template <typename T>
class FileTree {
public:
    using Tp = FileTreeNode<T>;
    using List = typename Tp::List;
    using iterator = typename Tp::iterator;

    explicit FileTree()
        : _root({}, List{})
        , _it(_root.getList().end())
    {}

    template <typename U, 
        typename = std::enable_if_t<std::is_same_v<U, Tp>>>
    iterator insert(std::optional<iterator> parentIt, U&& data) {
        auto& list = parentIt 
            ? (*parentIt)->getList() 
            : _root.getList();
        data._parentIt = parentIt;
        list.emplace_back(std::forward<U>(data));
        auto res = --list.end();
        if (res->isList()) {
            auto& chList = res->getList();
            for (auto& cIt : chList) {
                cIt._parentIt = res;
                data._cnt += cIt._cnt;
            }
            // 记录非文件夹节点的数量
            _cnt += data._cnt;
        } else {
            // 如果播放队列没有歌曲, 那么更新一下; 前提是: 添加的不是文件夹
            if (_it == _root.getList().end()) {
                _it = res;
            }
            data._cnt = 1;
            ++_cnt;
        }
        return res;
    }

    template <typename U, 
        typename = std::enable_if_t<std::is_same_v<U, Tp>>>
    iterator insert(std::optional<iterator> parentIt, int idx, U&& data) {
        auto& list = parentIt 
            ? (*parentIt)->getList() 
            : _root.getList();
        data._parentIt = parentIt;
        auto res = list.emplace(
            std::next(list.begin(), idx),
            std::forward<U>(data)
        );
        if (res->isList()) {
            // 注意, 如果是列表, 那么他们的一层列表项的父节点是需要更新的
            auto& chList = res->getList();
            for (auto& cIt : chList) {
                cIt._parentIt = res;
                data._cnt += cIt._cnt;
            }
            _cnt += data._cnt;
        } else {
            // 如果播放队列没有歌曲, 那么更新一下; 前提是: 添加的不是文件夹
            if (_it == _root.getList().end()) {
                _it = res;
            }
            data._cnt = 1;
            ++_cnt;
        }
        return res;
    }

    void erase(std::optional<iterator> parentIt, iterator delIt) {
        auto& list = parentIt 
            ? (*parentIt)->getList() 
            : _root.getList();
        if (_it == delIt) {
            _it = _root.getList().end();
        }
        if (delIt->isList()) {
            _cnt -= delIt->_cnt;
        } else {
            --_cnt;
        }
        list.erase(delIt);
    }

    bool empty() const noexcept {
        return _root.getList().empty();
    }

    /**
     * @brief 设置当前迭代器位置
     * @param it 
     */
    void setNowIt(iterator it) {
        _it = it;
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

    /**
     * @brief 设置当前播放歌曲为无效 (如果播放, 则会从头开始播放)
     */
    void setNull() {
        _it = _root.getList().end();
    }

protected:
    mutable Tp _root;
    iterator _it;
    std::size_t _cnt; // 只记录文件节点
};

} // namespace HX

#endif // !_HX_FILE_TREE_H_