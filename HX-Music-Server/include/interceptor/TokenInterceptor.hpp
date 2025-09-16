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

#include <api/Api.hpp>

#include <config/DbPath.hpp>
#include <token/TokenApi.hpp>
#include <utils/Timestamp.hpp>
#include <dao/UserDAO.hpp>
#include <dao/MemoryDAOPool.hpp>

#include <api/ApiMacro.hpp>

namespace HX {

// 凭证内容
struct TokenData {
    uint64_t userId;    // 用户 Id
    int64_t beginTime;  // 凭证生效时间: 毫秒级Unix时间戳
    int64_t endTime;    // 凭证失效时间: 毫秒级Unix时间戳
};

/**
 * @brief 凭证拦截器
 */
template <PermissionEnum Permission = PermissionEnum::ReadOnlyUser>
struct TokenInterceptor {
    // 凭证的 key
    inline static constexpr std::string_view HttpHeadTokenKay = "HX-Token";

    // 凭证的持续时间: ms
    inline static constexpr int64_t effectiveDuration = 24 * 60 * 60 * 1000; // 一天

    using Request = net::Request;
    using Response = net::Response;

    decltype(dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>()) userDAO
        = dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>();

    coroutine::Task<bool> before(Request& req, Response& res) {
        auto& head = req.getHeaders();
        auto it = head.find(HttpHeadTokenKay);
        if (it == head.end()) {
            co_await api::setJsonError("请携带凭证", res).sendRes();
            co_return false;
        }
        auto& tokenApi = token::TokenApi::get();
        bool ans = true;
        co_await api::coTryCatch([&] CO_FUNC {
            auto tokenData = tokenApi.fromToken<TokenData>(it->second);
            if (auto now = utils::Timestamp::getTimestamp<std::chrono::milliseconds>();
                now > tokenData.endTime || now < tokenData.beginTime
            ) {
                co_await api::setJsonError("凭证失效", res).sendRes();
                ans = false;
            } else if (userDAO->at(tokenData.userId).permissionLevel < Permission) {
                co_await api::setJsonError("权限不足", res).sendRes();
                ans = false;
            }
            co_return;
        }, [&] CO_FUNC {
            ans = false;
            co_await api::setJsonError("错误凭证", res).sendRes();
        });
        co_return ans;
    }
};

} // namespace HX

#include <api/UnApiMacro.hpp>