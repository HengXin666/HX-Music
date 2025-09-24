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

#include <HXLibs/net/client/HttpClientPool.hpp>

#include <config/Token.hpp>

namespace HX {

struct NetSingleton {
    inline static constexpr std::size_t CliCnt = 4;

    static NetSingleton& get() {
        static NetSingleton net{};
        return net;
    }

    container::FutureResult<container::Try<net::ResponseData>> getReq(std::string url) {
        log::hxLog.debug("http -> GET:", _backendUrl + url);
        return _cliPool.get(
            _backendUrl + std::move(url),
            {{std::string{config::HttpHeadTokenKay}, _token}}
        );
    }

    container::FutureResult<container::Try<net::ResponseData>> delReq(std::string url) {
        log::hxLog.debug("http -> DEL:", _backendUrl + url);
        return _cliPool.requst<net::DEL>(
            _backendUrl + std::move(url),
            {{std::string{config::HttpHeadTokenKay}, _token}}
        );
    }

    container::FutureResult<container::Try<net::ResponseData>> postReq(
        std::string url,
        std::string body,
        net::HttpContentType contentType = net::HttpContentType::Json
    ) {
        log::hxLog.debug("http -> POST:", _backendUrl + url);
        return _cliPool.post(
            _backendUrl + std::move(url),
            std::move(body),
            contentType,
            {{std::string{config::HttpHeadTokenKay}, _token}}
        );
    }

    template <typename T>
    container::FutureResult<container::Try<net::ResponseData>> postReq(std::string url, T&& data) {
        std::string json;
        reflection::toJson(data, json);
        log::hxLog.debug("http -> POST:", _backendUrl + url, "\njson:", json);
        return _cliPool.post(
            _backendUrl + std::move(url),
            std::move(json),
            net::HttpContentType::Json,
            {{std::string{config::HttpHeadTokenKay}, _token}}
        );
    }

    template <net::HttpMethod Method>
    container::FutureResult<container::Try<net::ResponseData>> pushFile(std::string url, std::string filePath) {
        log::hxLog.debug("http -> uploadChunked:", Method, _backendUrl + url, "file:", filePath);
        return _cliPool.uploadChunked<Method>(
            std::move(url),
            std::move(filePath),
            net::HttpContentType::OctetStream,
            {{std::string{config::HttpHeadTokenKay}, _token}}
        );
    }

    template <
        typename Lambda, 
        typename Res = coroutine::AwaiterReturnValue<std::invoke_result_t<Lambda, net::WebSocketClient>>
    >
        requires(std::is_same_v<std::invoke_result_t<Lambda, net::WebSocketClient>, coroutine::Task<Res>>)
    container::FutureResult<container::Try<Res>> wsReq(std::string url, Lambda&& lambda) {
        auto removeHttp = _backendUrl;
        auto _url = "ws"
            + removeHttp.substr(removeHttp.find("http") + sizeof("http") - 1)
            + std::move(url);
        log::hxLog.debug("ws -> get:", _url);
        return _cliPool.wsLoop(
            std::move(_url),
            std::forward<Lambda>(lambda)
        );
    }

    void setBackendUrl(std::string const& url) {
        _backendUrl = url;
        for (std::size_t i = 0; i < CliCnt; ++i) {
            _cliPool.close();
        }
    }

    std::string const& getBackendUrl() const noexcept {
        return _backendUrl;
    }

    void setToken(std::string token) {
        _token = std::move(token);
    }

    std::string const& getToken() const noexcept {
        return _token;
    }
private:
    /// @brief 后端 URL, 必须以 http 开头
    std::string _backendUrl{"http://127.0.0.1:28205"};

    /// @brief 凭证
    std::string _token{};

    /// @brief Http 客户端
    decltype(net::HttpClientPool{0
        // net::HttpClientOptions<
        // decltype(utils::operator""_s<"600">())>{}
    }) _cliPool{
        CliCnt,
        net::HttpClientOptions<
            // decltype(utils::operator""_s<"600">())
        >{}
    };

    NetSingleton() = default;
    NetSingleton& operator=(NetSingleton&&) = delete;
};

} // namespace HX