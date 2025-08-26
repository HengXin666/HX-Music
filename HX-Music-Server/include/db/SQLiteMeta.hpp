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

/**
 * @brief 自定义序列化类型
 * @tparam T 
 */
template <typename T>
struct SQLiteSqlType {
    inline static constexpr bool _hx_Val = true;

    // 序列化方法
    static constexpr std::string bind(T const& t) noexcept {
        return {};
    }

    // 反序列化方法
    static constexpr T columnType(std::string_view str) {
        return {};
    }
};

template <typename T>
constexpr bool isSQLiteSqlTypeVal = !requires { SQLiteSqlType<T>::_hx_Val; };

/**
 * @brief 设置为主键
 * @tparam T 
 */
template <typename T>
struct PrimaryKey {
    // 主键期望为整数
    static_assert(std::is_integral_v<T>, "The expected primary key is an integer");
    using PrimaryKeyType = T;

    T val;

    operator T&() noexcept {
        return val;
    }
};

template <typename T>
constexpr bool isPrimaryKeyVal = requires { typename T::PrimaryKeyType; };

namespace internal {

template <typename T>
struct RemovePrimaryKeyTypeImpl {
    using Type = T;
};

template <typename T>
struct RemovePrimaryKeyTypeImpl<PrimaryKey<T>> {
    using Type = typename PrimaryKey<T>::PrimaryKeyType;
};

} // namespace internal

/**
 * @brief 去除主键类型包装
 * @tparam T 
 */
template <typename T>
using RemovePrimaryKeyType = internal::RemovePrimaryKeyTypeImpl<T>::Type;

} // namespace HX::db