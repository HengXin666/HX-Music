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

#include <api/ApiMacro.hpp>
#include <api/Api.hpp>

#include <dao/MusicDAO.hpp>
#include <dao/MemoryDAOPool.hpp>

namespace HX {

HX_SERVER_API_BEGIN(CoverApi) {
    auto musicDAO 
        = dao::MemoryDAOPool::get<MusicDAO, "./file/db/music.db">();
    HX_ENDPOINT_BEGIN
        .addEndpoint<GET, HEAD>("/cover/select/{id}", [=] ENDPOINT {
            auto idStrView = req.getPathParam(0);
            MusicDAO::PrimaryKeyType id{};
            co_await api::coTryCatch([&] CO_FUNC {
                reflection::fromJson(id, idStrView);
                log::hxLog.debug("封面发送中...", id);
                co_await res.useRangeTransferFile(
                    req.getRangeRequestView(),
                    "./file/cover/" + std::to_string(id) + musicDAO->at(id).coverSuffix
                );
                log::hxLog.debug("封面发送完成!", id);
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌曲id不存在 或者 路径错误", res).sendRes();
            });
        })
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>