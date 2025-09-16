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
    using PrimaryKeyType = db::RemovePrimaryKeyType<meta::remove_cvref_t<decltype(
        std::get<
            db::GetFirstPrimaryKeyIndex<T>
        >(reflection::internal::getObjTie(std::declval<T>()))
    )>>;

    using MapType = std::map<db::GetFirstPrimaryKeyType<T>, T>;

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

    ThreadSafeInMemoryDAO& operator=(ThreadSafeInMemoryDAO&&) noexcept = delete;

    template <typename U>
        requires (std::convertible_to<U, T>)
    const T& add(U&& u) {
        std::unique_lock _{_mtx};
        auto id = _db.insert(u);
        db::getFirstPrimaryKeyRef<T>(u) = id;
        auto [it, ok] = _map.emplace(id, std::forward<U>(u));
        return it->second;
    }

    template <bool IsMustSucceed = false, typename U>
        requires (std::convertible_to<U, T>)
    const T& update(U&& u) {
        using namespace std::string_literals;
        std::unique_lock _{_mtx};
        auto id = db::getFirstPrimaryKeyRef<T>(u);
        _db.updateBy(u, ("where "s
                        += reflection::getMembersNames<T>()[db::GetFirstPrimaryKeyIndex<T>])
                        += " = ?")
            .bind(id)
            .execOnThrow();
        if constexpr (IsMustSucceed) {
            if (_db.lastLineChange() == 0) [[unlikely]] {
                throw std::runtime_error{"update By KeyId Fail: The data is" + log::formatString(u)};
            }
        }
        return _map[id] = std::forward<U>(u);
    }

    void del(PrimaryKeyType id) {
        using namespace std::string_literals;
        std::unique_lock _{_mtx};
        _db.deleteBy<T>(("where "s
                        += reflection::getMembersNames<T>()[db::GetFirstPrimaryKeyIndex<T>])
                        += " = ?")
            .bind(id)
            .execOnThrow();
        _map.erase(id);
    }

    const T& at(PrimaryKeyType id) const {
        std::shared_lock _{_mtx};
        return _map.at(id);
    }

    template <typename Lambda>
    decltype(auto) uniqueLock(Lambda&& lambda) const {
        std::unique_lock _{_mtx};
        return lambda();
    }

    template <typename Lambda>
    decltype(auto) sharedLock(Lambda&& lambda) const {
        std::shared_lock _{_mtx};
        return lambda();
    }

    template <typename Lambda, typename Res = std::invoke_result_t<Lambda, MapType const&>>
    Res lockSelect(Lambda&& lambda) const noexcept(noexcept(lambda(_map))) {
        std::shared_lock _{_mtx};
        return lambda(_map);
    }
private:
    db::SQLiteDB _db;
    MapType _map;
    mutable std::shared_mutex _mtx;
};

} // namespace HX::dao