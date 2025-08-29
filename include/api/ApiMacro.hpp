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
#include <HXLibs/reflection/json/JsonWrite.hpp>

/**
 * @brief 定义服务器端点 BEGIN
 */
#define HX_ENDPOINT_BEGIN bool HX_JOIN(_hx_EndpointName_, __LINE__) = [&]() {  \
        _server
/**
 * @brief 定义服务器端点 END
 */
#define HX_ENDPOINT_END                                                        \
    ;                                                                          \
    return false;                                                              \
    }                                                                          \
    ()

/**
 * @brief 定义服务器端点类 BEGIN
 */
#define HX_SERVER_API_BEGIN(__ApiName__)                                       \
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
#define HX_SERVER_API_END                                                      \
    ();                                                                        \
    return false;                                                              \
    }                                                                          \
    ();                                                                        \
    }

#define CO_FUNC () -> coroutine::Task<>