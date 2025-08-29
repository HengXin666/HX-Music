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

#include <HXLibs/net/Api.hpp>
#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/macro/Join.hpp>

namespace HX::api {

/**
 * @brief 注册 Api 到服务端
 * @tparam T Api 类
 * @param server 
 * @return auto& 
 */
template <typename T>
inline auto& addApi(net::HttpServer& server) {
    T {server};
    return server;
}

template <typename T, typename Body>
    requires (requires (Body const& body) {
        { body.getBody() } -> std::convertible_to<std::string_view>;
    })
T getVO(Body const& body) {
    T t{};
    reflection::fromJson(t, body.getBody());
    return t;
}

template <typename T, typename Body>
    requires (requires (Body& body, std::string s) {
        { body.setBody(std::move(s)) };
    })
void setVO(T const& t, Body& body) {
    std::string s;
    reflection::toJson(t, s);
    body.setBody(std::move(s));
}

/**
 * @brief 自动化的把 VO 转换为 DO
 * @warning 要求 VO 内部实现了对应的转换
 * @note 考虑到一般 VO 是 DO 的子集, 这样的自动转换是安全的; 但是反之就可能会泄漏. 因此 DO -> VO 得显式写明.
 * @tparam T 
 * @tparam U 
 * @return T 
 */
template <typename T, typename U>
    requires (requires (U&& u) {
        { std::forward<U>(u).operator T() } -> std::convertible_to<T>;
    })
T toDO(U&& u) noexcept {
    return std::forward<U>(u).operator T();
}

template <typename MainLambda, typename ErrLambda>
coroutine::Task<> coTryCatch(MainLambda main, ErrLambda err) {
    bool isErr = false;
    try {
        co_await main();
    } catch (...) {
        isErr = true;
    }
    if (isErr) [[unlikely]] {
        co_await err();
    }
}

} // namespace HX::api