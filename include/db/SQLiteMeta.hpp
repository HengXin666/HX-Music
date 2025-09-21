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

#include <HXLibs/reflection/MemberName.hpp>

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

    operator T&() noexcept { return val; }
    operator const T&() const noexcept { return val; }
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

template <typename T, typename... Args>
struct GetFirstPrimaryKeyTypeImpl {
    using Type = GetFirstPrimaryKeyTypeImpl<Args...>::Type;
};

template <typename T, typename... Args>
struct GetFirstPrimaryKeyTypeImpl<db::PrimaryKey<T>, Args...> {
    using Type = db::PrimaryKey<T>::PrimaryKeyType;
};

template <>
struct GetFirstPrimaryKeyTypeImpl<void> {
    using Type = void;
};

template <typename Idx, typename... Args>
struct GetFirstPrimaryKeyIndex;

template <std::size_t I, std::size_t... Is, typename T, typename... Args>
struct GetFirstPrimaryKeyIndex<std::index_sequence<I, Is...>, T, Args...> {
    inline static constexpr std::size_t Val 
        = GetFirstPrimaryKeyIndex<std::index_sequence<Is...>, Args...>::Val;
};

template <std::size_t I, std::size_t... Is, typename T, typename... Args>
struct GetFirstPrimaryKeyIndex<std::index_sequence<I, Is...>, db::PrimaryKey<T>, Args...> {
    inline static constexpr std::size_t Val = I;
};

} // namespace internal

/**
 * @brief 去除主键类型包装
 * @tparam T 
 */
template <typename T>
using RemovePrimaryKeyType = internal::RemovePrimaryKeyTypeImpl<T>::Type;

/**
 * @brief 获取 T 类 中的表示主键成员的类型
 * @tparam T 
 */
template <typename T>
using GetFirstPrimaryKeyType = decltype([]() constexpr {
    constexpr auto tp = reflection::internal::getStaticObjPtrTuple<T>();
    return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
        return typename internal::GetFirstPrimaryKeyTypeImpl<
            meta::remove_cvref_t<decltype(*std::get<Is>(tp))>..., void
        >::Type {};
    }(std::make_index_sequence<std::tuple_size_v<decltype(tp)>>{});
}());

/**
 * @brief 获取主键在类对象中的索引 (从 0 开始)
 * @tparam T 
 */
template <typename T>
constexpr std::size_t GetFirstPrimaryKeyIndex = [](){
    constexpr auto tp = reflection::internal::getStaticObjPtrTuple<T>();
    return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
        return internal::GetFirstPrimaryKeyIndex<
            std::make_index_sequence<std::tuple_size_v<decltype(tp)>>,
            meta::remove_cvref_t<decltype(*std::get<Is>(tp))>...
        >::Val;
    }(std::make_index_sequence<std::tuple_size_v<decltype(tp)>>{});
}();

/**
 * @brief 获取主键引用
 * @tparam T 
 * @param t 
 * @return constexpr auto& 
 */
template <typename T>
inline constexpr auto& getFirstPrimaryKeyRef(T& t) noexcept {
    auto tr = reflection::internal::getObjTie(t);
    return std::get<GetFirstPrimaryKeyIndex<T>>(tr).val;
}

} // namespace HX::db