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

#include <interceptor/TokenInterceptor.hpp>
#include <dao/UserDAO.hpp>

#include <api/ApiMacro.hpp>

namespace HX {

HX_SERVER_API_BEGIN(UserApi) {
    
    auto userDAO
        = dao::MemoryDAOPool::get<UserDAO, "./file/db/user.db">();

    HX_ENDPOINT_BEGIN
        .addEndpoint<GET>("/login/test", [] ENDPOINT {
            co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
        }, TokenInterceptor{})
    HX_ENDPOINT_END;

} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>