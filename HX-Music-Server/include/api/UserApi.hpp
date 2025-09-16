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
#include <dao/MemoryDAOPool.hpp>

#include <config/DbPath.hpp>
#include <token/TokenApi.hpp>
#include <interceptor/TokenInterceptor.hpp>
#include <dao/UserDAO.hpp>
#include <pojo/vo/UserAddVO.hpp>
#include <pojo/vo/UserLoginVO.hpp>
#include <utils/Uuid.hpp>
#include <utils/MD5.hpp>

#include <api/ApiMacro.hpp>

namespace HX {

HX_SERVER_API_BEGIN(UserApi) {
    
    auto userDAO
        = dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>();

    HX_ENDPOINT_BEGIN
        // 内部测试接口
        .addEndpoint<GET>("/user/test", [] ENDPOINT {
            co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
        }, TokenInterceptor{})
        // 注册接口: 仅管理员可以创建用户
        .addEndpoint<POST>("/user/add", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto userAddVO = co_await api::getVO<UserAddVO>(req);
                if (userAddVO.password.empty()) [[unlikely]] {
                    co_return co_await api::setJsonError("密码不能为空", res).sendRes();
                }
                if (userDAO->atName(userAddVO.name)) [[unlikely]] {
                    co_return co_await api::setJsonError("用户名已存在", res).sendRes();
                }
                // 密码加盐
                auto salt = utils::Uuid::makeV4();
                userAddVO.password += salt;
                // 计算 MD5
                userDAO->add<UserDO>({
                    {},
                    std::move(userAddVO.name),
                    "点击输入文本",
                    std::move(salt),
                    utils::md5(userAddVO.password),
                    {},
                    {},
                    userAddVO.permissionLevel
                });
                co_return co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("数据非法", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::Administrator>{})
        // 登录接口, 返回凭证
        .addEndpoint<POST>("/user/login", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto userLoginVO = co_await api::getVO<UserLoginVO>(req);
                auto idOpt = userDAO->atName(userLoginVO.name);
                if (!idOpt) {
                    co_return co_await api::setJsonError("用户名不存在", res).sendRes();
                }
                auto const& userDO = userDAO->at(*idOpt);
                userLoginVO.password += userDO.salt;
                if (utils::md5(userLoginVO.password) != userDO.password) {
                    co_return co_await api::setJsonError("用户名或密码错误", res).sendRes();
                }
                // 计算凭证, 并且返回
                auto nowTime = utils::Timestamp::getTimestamp();
                co_return co_await api::setJsonSucceed(
                    token::TokenApi::get().toToken<TokenData>({
                        userDO.id,
                        nowTime,
                        nowTime
                    }), res
                ).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("数据非法", res).sendRes();
            });
        })
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>