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

#include <pojo/vo/JsonVO.hpp>

namespace HX::api {

template <typename T>
constexpr vo::JsonVO<T> succeed(T&& data) {
    return vo::JsonVO<T>::succeed(std::forward<T>(data));
}

template <typename T = std::string>
constexpr vo::JsonVO<T> error(std::string msg) {
    return vo::JsonVO<T>::error(std::move(msg));
}

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

template <typename T>
coroutine::Task<T> getVO(net::Request& req) {
    T t{};
    reflection::fromJson(t,  co_await req.parseBody());
    co_return t;
}

template <typename T, typename Body>
    requires (requires (Body const& body) {
        { (body.body) } -> std::convertible_to<std::string_view>;
    })
T getVO(Body const& body) {
    T t{};
    reflection::fromJson(t, body.body);
    return t;
}

template <typename Body>
    requires (requires (Body const& body) {
        { (body.body) } -> std::convertible_to<std::string_view>;
    })
inline void throwVoMsg(Body&& body) {
    auto vo = api::getVO<vo::JsonVO<std::string>>(std::forward<Body>(body));
    throw std::runtime_error{std::move(vo.msg)};
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
 * @brief 设置 JSON 类型 和 响应码 200
 * @param res 
 * @return auto& 
 */
template <typename T>
inline auto& setJsonSucceed(T&& data, net::Response& res) noexcept {
    setVO(api::succeed<T>(std::forward<T>(data)), res);
    return res.setResLine(net::Status::CODE_200)
              .setContentType(net::JSON);
}

/**
 * @brief 设置 JSON 类型 和 响应码 400
 * @param res 
 * @return auto& 
 */
inline auto& setJsonError(std::string msg, net::Response& res) noexcept {
    setVO(api::error(std::move(msg)), res);
    return res.setResLine(net::Status::CODE_400)
              .setContentType(net::JSON);
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

/**
 * @brief [客户端使用] 快速检查 Try<ResponseData> & 状态码 是否合法, 否则抛出异常
 * @tparam T 应该为 vo::JsonVO<U>
 * @param t 
 * @return T = vo::JsonVO<U>
 */
template <typename T>
T checkTryAndStatus(container::Try<net::ResponseData> t) {
    if (!t) [[unlikely]] {
        throw std::runtime_error{"check: " + t.what()};
    } else if (t.get().status / 100 != 2) [[unlikely]] {
        throw std::runtime_error{"check: Status Err"};
    }
    return getVO<T>(t.move());
}

/**
 * @brief [客户端使用] 快速检查 vo::JsonVO<T> 是否合法, 否则抛出异常
 * @tparam T 
 * @param vo 
 * @return T JsonVO的内部存储的data
 */
template <typename T>
T checkJsonVO(vo::JsonVO<T> vo) {
    if (vo.code != vo::VOCode::OK) [[unlikely]] {
        throw std::runtime_error{std::move(vo.msg)};
    }
    return std::move(*vo.data);
}

/**
 * @brief [客户端使用] 快速检查 Try<ResponseData> & 状态码 & vo::JsonVO<T> 是否合法, 否则抛出异常
 * @tparam T 
 * @param t 
 * @return T 
 */
template <typename T>
T checkTryAndStatusAndJsonVO(container::Try<net::ResponseData> t) {
    return checkJsonVO(checkTryAndStatus<vo::JsonVO<T>>(t));
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