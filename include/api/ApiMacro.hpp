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
#include <HXLibs/reflection/json/JsonWrite.hpp>
#include <HXLibs/macro/Join.hpp>

/**
 * @brief 定义服务器端点 BEGIN
 */
#define HX_EndpointBegin bool HX_JOIN(_hx_EndpointName_, __LINE__) = [this]() {  \
        _server
/**
 * @brief 定义服务器端点 END
 */
#define HX_EndpointEnd                                                         \
    ;                                                                          \
    return false;                                                              \
    }                                                                          \
    ()

/**
 * @brief 定义服务器端点类 BEGIN
 */
#define HX_ServerApiBegin(__ApiName__)                                         \
    class __ApiName__ {                                                        \
    public:                                                                    \
        __ApiName__(net::HttpServer& server) : _server{server} {               \
        }                                                                      \
                                                                               \
    private:                                                                   \
        net::HttpServer& _server;                                              \
        bool _hx_init_ = [this]() { \
            using namespace HX::net; \
            [&]()
/**
 * @brief 定义服务器端点类 END
 */
#define HX_ServerApiEnd                                                        \
    ();                                                                        \
    return false;                                                              \
    }                                                                          \
    ();                                                                        \
    }

/**
 * @brief 注册 API 到服务端
 */
#define HX_ServerAddApi(__Server__, __ApiName__) __ApiName__{__Server__};

namespace HX::api {
    
template <typename T,typename Body>
    requires (requires (Body const& body) {
        { body.getBody() } -> std::convertible_to<std::string_view>;
    })
T getVO(Body const& body) {
    T t{};
    reflection::fromJson(t, body.getBody());
    return t;
}

template <typename T,typename Body>
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
    return T{std::forward<U>(u)};
}

} // namespace HX::api