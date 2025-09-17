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
#include <config/Token.hpp>
#include <token/TokenApi.hpp>
#include <utils/Timestamp.hpp>
#include <dao/UserDAO.hpp>
#include <dao/MemoryDAOPool.hpp>

#include <api/ApiMacro.hpp>

namespace HX {

// 凭证内容
struct TokenData {
    std::string loginUuid;  // 登录 Id, 用于验证登录是否合法 (比如修改密码后, 用户会退出登录)
    uint64_t userId;        // 用户 Id
    int64_t beginTime;      // 凭证生效时间: 毫秒级Unix时间戳
    int64_t endTime;        // 凭证失效时间: 毫秒级Unix时间戳
};

/**
 * @brief 凭证拦截器
 */
template <PermissionEnum Permission = PermissionEnum::ReadOnlyUser>
struct TokenInterceptor {
    using Request = net::Request;
    using Response = net::Response;

    decltype(dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>()) userDAO
        = dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>();

    coroutine::Task<bool> before(Request& req, Response& res) {
        auto& head = req.getHeaders();
        auto it = head.find(config::HttpHeadTokenKay);
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
            } else if (auto const& userDO = userDAO->at(tokenData.userId);
                userDO.permissionLevel < Permission
            ) {
                co_await api::setJsonError("权限不足", res).sendRes();
                ans = false;
            } else if (userDO.loggedInUuid != tokenData.loginUuid) {
                co_await api::setJsonError("凭证失效", res).sendRes();
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

/**
 * @brief 获取凭证
 * @warning 内部默认凭证存在, 即在此之前必须 通过 TokenInterceptor 的拦截. 否则不是期望的.
 * @param req 
 * @return TokenData 
 */
inline TokenData getTokenData(net::Request& req) {
    return token::TokenApi::get().fromToken<TokenData>(
        req.getHeaders().find(config::HttpHeadTokenKay)->second
    );
}

} // namespace HX

#include <api/UnApiMacro.hpp>