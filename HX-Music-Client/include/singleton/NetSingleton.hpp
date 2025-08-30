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

    std::string const& getBackendUrl() const noexcept {
        return _backendUrl;
    }

private:
    /// @brief 后端 URL
    std::string _backendUrl{"http://127.0.0.1:28205"};

    /// @brief Http 客户端
    decltype(net::HttpClient {}) _client{net::HttpClientOptions{}, 4};

    NetSingleton() = default;
    NetSingleton& operator=(NetSingleton&&) = delete;
};

} // namespace HX