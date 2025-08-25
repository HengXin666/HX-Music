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

#include <string>
#include <string_view>

namespace HX::db {

template <typename T>
struct SQLiteSqlType {
    inline static constexpr bool _hx_Val = true;

    static constexpr std::string bind(T const& t) noexcept {
        return {};
    }

    static constexpr T columnType(std::string_view str) {
        return {};
    }
};

template <typename T>
constexpr bool isSQLiteSqlTypeVal = !requires { SQLiteSqlType<T>::_hx_Val; };

} // namespace HX::db