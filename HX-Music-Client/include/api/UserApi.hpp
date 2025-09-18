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

#include <singleton/NetSingleton.hpp>
#include <singleton/GlobalSingleton.hpp>

#include <api/Api.hpp>
#include <pojo/vo/UserLoginVO.hpp>

namespace HX {

/**
 * @brief 用户相关请求 API
 */
struct UserApi {
    // 登录
    static container::FutureResult<> loginReq(std::string name, std::string passwd) {
        return NetSingleton::get().postReq("/user/login", UserLoginVO{
            std::move(name),
            std::move(passwd)
        }).thenTry([](container::Try<net::ResponseData> t) {
            auto token = api::checkTryAndStatusAndJsonVO<std::string>(std::move(t));
            log::hxLog.info("获取到凭证", token);
            NetSingleton::get().setToken(
                GlobalSingleton::get().musicConfig.token = std::move(token)
            );
        });
    }
};

} // namespace HX