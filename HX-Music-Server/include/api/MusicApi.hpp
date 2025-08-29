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
#include <utils/DirFor.hpp>

#include <HXLibs/reflection/json/JsonRead.hpp>

namespace HX {

/**
 * @brief 音乐播放 相关服务 API
 */
HX_SERVER_API_BEGIN(MusicApi) {
    auto musicDAO = std::make_shared<MusicDAO>(
        db::SQLiteDB{"./file/db/music.db"}
    );
    HX_ENDPOINT_BEGIN
        // 断点续传下载音乐
        .addEndpoint<GET, HEAD>("/music/download/{id}", [=] ENDPOINT {
            using namespace std::string_literals;
            log::hxLog.debug("请求 Path:", req.getReqPath());
            co_await api::coTryCatch([&] CO_FUNC {
                auto idStrView = req.getPathParam(0);
                MusicDAO::PrimaryKeyType id{};
                reflection::fromJson(id, idStrView);
                std::string_view path;
                path = musicDAO->at(id).path;
                co_await res.useRangeTransferFile(
                     req.getRangeRequestView(),
                     "./file/music/"s += path
                );
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "id 不正确!")
                            .sendRes();
            });
        })
        // 扫描服务端音乐
        .addEndpoint<GET>("/music/runScan", [=] ENDPOINT {
            std::size_t cnt = 0;
            utils::traverseDirectory("./file/music", {},
                [&](const std::filesystem::path& relativePath) {
                std::string path = relativePath.string();
                if (!musicDAO->isExist(path)) {
                    log::hxLog.info("新增歌曲:", path);
                    musicDAO->add<MusicDO>({
                        {},
                        std::move(path)
                    });
                    ++cnt;
                }
            });
            co_await res.setStatusAndContent(
                Status::CODE_200,
                "OK: 扫描完成, 新增 " + std::to_string(cnt) + " 首音乐!")
                        .sendRes();
        })
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>