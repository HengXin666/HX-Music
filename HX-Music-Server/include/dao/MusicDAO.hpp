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

#include <unordered_set>

#include <dao/ThreadSafeInMemoryDAO.hpp>
#include <pojo/do/MusicDO.hpp>

namespace HX {

struct MusicDAO : public dao::ThreadSafeInMemoryDAO<MusicDO> {
    using T = MusicDO;
    using Base = dao::ThreadSafeInMemoryDAO<MusicDO>;
    using Base::Base;

    MusicDAO(db::SQLiteDB db)
        : Base{std::move(db)}
    {
        Base::lockSelect([&](auto const& mp) {
            for (auto const& it: mp) {
                _pathSet.insert(it.second.path);
            }
        });
    }

    template <typename U>
    T add(U&& u) {
        auto t = Base::add(std::forward<U>(u));
        Base::uniqueLock([&] {
            _pathSet.insert(t.path);
        });
        return t;
    }

    template <typename U>
    T update(U&& u) {
        std::string oldPath = Base::at(u.id).path;
        const auto& t = Base::update(std::forward<U>(u));
        Base::uniqueLock([&] {
            if (oldPath != t.path) {
                _pathSet.erase(oldPath);
                _pathSet.insert(t.path);
            }
        });
        return t;
    }

    void del(PrimaryKeyType id) {
        std::string oldPath = Base::at(id).path;
        Base::uniqueLock([&] {
            _pathSet.erase(oldPath);
        });
        Base::del(id);
    }

    /**
     * @brief 判断 `路径` 对应的文件是否已经记录过了 
     * @param path 相对路径
     * @return true 已经记录过了
     * @return false 未记录
     */
    bool isExist(std::string_view path) const noexcept {
        return Base::sharedLock([&] {
            return _pathSet.contains(path);
        });
    }
private:
    // 记录路径
    std::unordered_set<std::string_view> _pathSet;
};

} // namespace HX
