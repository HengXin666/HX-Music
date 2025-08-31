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
#include <dao/MemoryDAOPool.hpp>

#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/utils/FileUtils.hpp>

#include <dao/MusicDAO.hpp>
#include <pojo/vo/MusicVO.hpp>
#include <utils/DirFor.hpp>
#include <utils/MusicInfo.hpp>

namespace HX {

/**
 * @brief 音乐播放 相关服务 API
 */
HX_SERVER_API_BEGIN(MusicApi) {
    auto musicDAO 
        = dao::MemoryDAOPool::get<MusicDAO, "./file/db/music.db">();
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
                api::setVO(api::error(
                    "歌曲id不存在"
                ), res);
                co_await api::setJsonError(res).sendRes();
            });
        })
        // 扫描服务端音乐
        .addEndpoint<GET>("/music/runScan", [=] ENDPOINT {
            std::size_t cnt = 0;
            coroutine::EventLoop loop;
            utils::traverseDirectory("./file/music", {},
                [&](const std::filesystem::path& relativePath) {
                std::string path = relativePath.string();
                std::filesystem::path fullPath = std::filesystem::path{"./file/music"} / relativePath;
                if (!std::filesystem::is_directory(fullPath) && !musicDAO->isExist(path)) {
                    log::hxLog.info("新增歌曲:", path);
                    MusicInfo info{fullPath};
                    auto imgOpt = info.getAlbumArtAdvanced();
                    auto const& dao = musicDAO->add<MusicDO>({
                        {},
                        std::move(path),
                        info.getTitle(),
                        info.getArtistList(),
                        info.getAlbum(),
                        static_cast<uint64_t>(info.getLengthInMilliseconds()),
                        imgOpt ? imgOpt->type : ""
                    });  
                    if (imgOpt) {
                        auto img = *imgOpt;
                        utils::AsyncFile file{loop};
                        file.syncOpen(
                            "./file/cover/" + std::to_string(dao.id) + std::move(img.type)
                        );
                        file.syncWrite(img.buf);
                        file.syncClose();
                    }
                    ++cnt;
                }
            });
            co_await res.setStatusAndContent(
                Status::CODE_200,
                "OK: 扫描完成, 新增 " + std::to_string(cnt) + " 首音乐!")
                        .sendRes();
        })
        // 获取音乐信息
        .addEndpoint<GET>("/music/info/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto idStrView = req.getPathParam(0);
                MusicDAO::PrimaryKeyType id{};
                reflection::fromJson(id, idStrView);
                auto const& musicDO = musicDAO->at(id);
                auto vo = api::succeed<MusicVO>({
                    musicDO.id,
                    musicDO.path,
                    musicDO.musicName,
                    musicDO.singers,
                    musicDO.musicAlbum,
                    musicDO.millisecondsLen
                });
                api::setVO(vo, res);
                co_await api::setJsonSucceed(res).sendRes();
            }, [&] CO_FUNC {
                api::setVO(api::error<MusicVO>("歌曲 ID 不存在"), res);
                co_await api::setJsonError(res).sendRes();
            });
        })
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>