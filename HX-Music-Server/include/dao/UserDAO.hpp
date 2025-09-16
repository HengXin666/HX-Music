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

#include <dao/ThreadSafeInMemoryDAO.hpp>
#include <pojo/do/UserDO.hpp>

namespace HX {

struct UserDAO : public dao::ThreadSafeInMemoryDAO<UserDO> {
    using T = UserDO;
    using Base = dao::ThreadSafeInMemoryDAO<UserDO>;
    using Base::Base;

    UserDAO(db::SQLiteDB db)
        : Base{std::move(db)}
    {
        Base::lockSelect([&](UserDAO::MapType const& mp) {
            for (auto const& [id, data] : mp) {
                _nameMapId.emplace(data.name, id);
            }
        });
    }

    template <typename U>
    const T& add(U&& u) {
        const auto& t = Base::add(std::forward<U>(u));
        Base::uniqueLock([&] {
            _nameMapId.emplace(u.name, u.id);
        });
        return t;
    }

    template <typename U>
    const T& update(U&& u) {
        std::string name = Base::at(u.id).name;
        const auto& t = Base::update(std::forward<U>(u));
        Base::uniqueLock([&] {
            if (name != t.name) {
                _nameMapId.erase(name);
                _nameMapId.emplace(t.name, t.id);
            }
        });
        return t;
    }

    void del(PrimaryKeyType id) {
        std::string name = Base::at(id).name;
        Base::uniqueLock([&] {
            _nameMapId.erase(name);
        });
        Base::del(id);
    }

    std::optional<uint64_t> atName(std::string_view name) {
        return Base::sharedLock([&]() -> std::optional<uint64_t> {
            auto it = _nameMapId.find(name);
            if (it == _nameMapId.end()) {
                return {};
            }
            return it->second;
        });
    }
private:
    // 用户名 -> 用户 Id 映射
    std::map<std::string, uint64_t, std::less<>> _nameMapId;
};

} // namespace HX