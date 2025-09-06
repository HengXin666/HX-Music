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

#include <dao/MusicDAO.hpp>
#include <pybind/ToKaRaOKAss.hpp>

namespace HX {

/**
 * @brief 歌词相关API
 */
HX_SERVER_API_BEGIN(LyricsApi) {
    auto musicDAO 
        = dao::MemoryDAOPool::get<MusicDAO, "./file/db/music.db">();
    auto toKaRaOKAssPtr = getToKaRaOKAssPtr();
    HX_ENDPOINT_BEGIN
        // 获取 ass 歌词
        .addEndpoint<GET, HEAD>("/lyrics/ass/select/{id}", [] ENDPOINT {
            auto idStrView = req.getPathParam(0);
            MusicDAO::PrimaryKeyType id{};
            co_await api::coTryCatch([&] CO_FUNC {
                reflection::fromJson(id, idStrView);
                log::hxLog.debug("歌词发送中...", id);
                co_await res.useRangeTransferFile(
                    req.getRangeRequestView(),
                    "./file/lyrics/ass/" + std::to_string(id) + ".ass"
                );
                log::hxLog.debug("歌词发送完成!", id);
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌曲id不存在 或者 路径错误", res).sendRes();
            });
        })
        // 网络爬取歌曲歌词并且日语注音卡拉ok化
        .addEndpoint<GET, POST>("/lyrics/ass/karaok/{id}", [=] ENDPOINT {
            auto idStrView = req.getPathParam(0);
            MusicDAO::PrimaryKeyType id{};
            co_await api::coTryCatch([&] CO_FUNC {
                reflection::fromJson(id, idStrView);
                auto const& musicDO = musicDAO->at(id);
                std::filesystem::path assPath{std::filesystem::current_path() / "file/lyrics/ass" / (std::string{idStrView} + ".ass")};
                log::hxLog.info("歌词查找:", std::filesystem::current_path() / "file/music" / musicDO.path, "->", assPath);
                try {
                    toKaRaOKAssPtr->findLyricsFromNet(
                        std::filesystem::current_path() / "file/music" / musicDO.path,
                        assPath
                    )
                     .doJapanesePhonetics(assPath)
                     .toTwoLineKaraokeStyle(assPath)
                     .callApplyKaraokeTemplateLua(assPath);
                    co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
                } catch (std::exception const& e) {
                    if (std::string_view{e.what()}.find("LyricsNotFoundError") == std::string_view::npos) {
                        throw;
                    }
                }
                co_await api::setJsonError("找不到符合条件的歌词...", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌曲id不存在 或者 路径错误", res).sendRes();
            });
        })
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>