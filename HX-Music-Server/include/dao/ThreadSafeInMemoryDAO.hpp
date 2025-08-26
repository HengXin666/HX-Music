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

#include <HXLibs/reflection/MemberName.hpp>

#include <db/SQLiteMeta.hpp>
#include <db/SQLiteDB.hpp>

namespace HX::dao {

/**
 * @brief 线程安全访问的, 数据缓存; 启动时候会把所有数据加载到内存, 日后全部访问基于内存; 仅增删改会同步一次到数据库.
 * @tparam T 
 */
template <typename T>
struct ThreadSafeInMemoryDAO {
    using PrimaryKeyType = db::RemovePrimaryKeyType<decltype(
        std::get<
            db::GetFirstPrimaryKeyIndex<T>
        >(reflection::internal::getObjTie(std::declval<T>()))
    )>;

    ThreadSafeInMemoryDAO(db::SQLiteDB db)
        : _db{std::move(db)}
        , _map{}
        , _mtx{}
    {
        _db.createDatabase<T>();
        auto res = _db.queryAll<T>();
        for (auto&& it : res) {
            auto id = db::getFirstPrimaryKeyRef<T>(it);
            _map.emplace(id, std::move(it));
        }
    }

    template <typename U>
    const T& add(U&& t) {
        std::unique_lock _{_mtx};
        auto id = _db.insert(t);
        db::getFirstPrimaryKeyRef<T>(t) = id;
        auto [it, ok] = _map.emplace(id, std::forward<U>(t));
        return it->second;
    }

    template <typename U>
    const T& update(U&& t) {
        using namespace std::string_literals;
        std::unique_lock _{_mtx};
        auto id = db::getFirstPrimaryKeyRef<T>(t);
        _db.updateBy(t, ("where "s
                        += reflection::getMembersNames<T>()[db::GetFirstPrimaryKeyIndex<T>])
                        += " = ?")
            .bind(id)
            .exec();
        return _map[id] = std::forward<U>(t);
    }

    void del(PrimaryKeyType id) {
        using namespace std::string_literals;
        std::unique_lock _{_mtx};
        _db.deleteBy<T>(("where "s
                        += reflection::getMembersNames<T>()[db::GetFirstPrimaryKeyIndex<T>])
                        += " = ?")
            .bind(id)
            .exec();
        _map.erase(id);
    }

    const T& at(PrimaryKeyType id) const {
        std::shared_lock _{_mtx};
        return _map.at(id);
    }

private:
    db::SQLiteDB _db;
    std::map<db::GetFirstPrimaryKeyType<T>, T> _map;
    mutable std::shared_mutex _mtx;
};

} // namespace HX::dao