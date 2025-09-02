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

#include <HXLibs/net/client/HttpClient.hpp>

namespace HX {

struct NetSingleton {
    static NetSingleton& get() {
        static NetSingleton net{};
        return net;
    }

    auto getReq(std::string url) {
        log::hxLog.debug("http -> GET:", _backendUrl + url);
        return _client.get(_backendUrl + std::move(url));
    }

    auto postReq(
        std::string url,
        std::string body,
        net::HttpContentType contentType = net::HttpContentType::Json
    ) {
        log::hxLog.debug("http -> POST:", _backendUrl + url);
        return _client.post(_backendUrl + std::move(url), std::move(body), contentType);
    }

    template <typename T>
    auto postReq(std::string url, T&& data) {
        std::string json;
        reflection::toJson(data, json);
        log::hxLog.debug("http -> POST:", _backendUrl + url, "\njson:", json);
        return _client.post(
            _backendUrl + std::move(url),
            std::move(json),
            net::HttpContentType::Json
        );
    }

    template <typename Lambda>
        requires(std::is_same_v<std::invoke_result_t<Lambda, net::WebSocketClient>, coroutine::Task<>>)
    auto wsReq(std::string url, Lambda&& lambda) {
        return _client.wsLoop(std::move(url), std::forward<Lambda>(lambda));
    }

    std::string const& getBackendUrl() const noexcept {
        return _backendUrl;
    }

private:
    /// @brief 后端 URL
    std::string _backendUrl{"http://127.0.0.1:28205"};

    /// @brief Http 客户端
    decltype(net::HttpClient{net::HttpClientOptions<
        decltype(utils::operator""_s<"600">())>{}}) _client{
        net::HttpClientOptions<decltype(utils::operator""_s<"600">())>{}, 4
    };

    NetSingleton() = default;
    NetSingleton& operator=(NetSingleton&&) = delete;
};

} // namespace HX