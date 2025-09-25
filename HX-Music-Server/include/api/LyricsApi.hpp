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
#include <dao/MusicDAO.hpp>
#include <interceptor/TokenInterceptor.hpp>
#include <pybind/ToKaRaOKAss.hpp>

#include <api/ApiMacro.hpp>
#include <pojo/vo/WsLyricsMsgVO.hpp>

namespace HX {

/**
 * @brief 歌词相关API
 */
HX_SERVER_API_BEGIN(LyricsApi) {
    auto musicDAO 
        = dao::MemoryDAOPool::get<MusicDAO, config::MusicDbPath>();
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
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
        // 网络爬取歌曲歌词并且日语注音卡拉ok化
        .addEndpoint<POST>("/lyrics/ass/karaok/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto id = req.getPathParam(0).to<MusicDAO::PrimaryKeyType>();
                auto musicDO = musicDAO->at(id);
                std::filesystem::path assPath{std::filesystem::current_path() / "file/lyrics/ass" / (std::string{req.getPathParam(0)} + ".ass")};
                log::hxLog.info("歌词查找:", std::filesystem::current_path() / "file/music" / musicDO.path, "->", assPath);
                try {
                    toKaRaOKAssPtr->_findLyricsFromNet(
                        std::filesystem::current_path() / "file/music" / musicDO.path,
                        assPath
                    )
                     ._doJapanesePhonetics(assPath)
                     ._toTwoLineKaraokeStyle(assPath)
                     ._callApplyKaraokeTemplateLua(assPath);
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
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 异步获取歌词接口
        .addEndpoint<WS>("/lyrics/ass/karaok/ws", [=] ENDPOINT {
            auto ws = co_await net::WebSocketFactory::accept(req, res);
            for (;;) {
                namespace fs = std::filesystem;
                WsLyricsMsgVO<> msgVO;
                co_await ws.recvJson(msgVO);
                // 爬取歌词
                const auto musicPath = musicDAO->at(msgVO.musicId, &MusicDO::path);
                fs::path assPath {
                    std::filesystem::current_path()
                    / "file/lyrics/ass"
                    / (std::to_string(msgVO.musicId) + ".ass")
                };
                switch (msgVO.type) {
                case WsLyricsMsgEnum::CrawlLyrics: {
                    bool isNotFind = false;
                    try {
                        co_await toKaRaOKAssPtr->findLyricsFromNet(
                            std::filesystem::current_path() / "file/music" / musicPath,
                            std::move(assPath)
                        ).via(req.getIO());
                        co_await ws.sendJson<WsLyricsMsgVO<std::string>>({
                            msgVO.musicId,
                            msgVO.type,
                            "ok"
                        });
                        break;
                    } catch (std::exception const& e) {
                        log::hxLog.error("爬取歌词 Err:", e.what());
                        isNotFind = std::string_view{e.what()}.find("LyricsNotFoundError") == std::string_view::npos;
                    }
                    co_await (isNotFind 
                        ? ws.sendJson<WsLyricsMsgVO<std::string>>({
                            msgVO.musicId,
                            msgVO.type,
                            "err: LyricsNotFoundError"
                        })
                        : ws.sendJson<WsLyricsMsgVO<std::string>>({
                            msgVO.musicId,
                            msgVO.type,
                            "err: unknown"
                        }));
                    break;
                }
                case WsLyricsMsgEnum::JpTranscription: {
                    // 日语注音
                    co_await toKaRaOKAssPtr->doJapanesePhonetics(
                        std::move(assPath)
                    ).via(req.getIO());
                    co_await ws.sendJson<WsLyricsMsgVO<std::string>>({
                        msgVO.musicId,
                        msgVO.type,
                        "ok"
                    });
                    break;
                }
                case WsLyricsMsgEnum::TwoLineKaraokeStyle: {
                    // 双行卡拉ok化
                    co_await toKaRaOKAssPtr->toTwoLineKaraokeStyle(
                        std::move(assPath)
                    ).via(req.getIO());
                    co_await ws.sendJson<WsLyricsMsgVO<std::string>>({
                        msgVO.musicId,
                        msgVO.type,
                        "ok"
                    });
                    break;
                }
                case WsLyricsMsgEnum::CallKaraokeTemplateLua: {
                    // 应用卡拉ok模板
                    co_await toKaRaOKAssPtr->callApplyKaraokeTemplateLua(
                        std::move(assPath)
                    ).via(req.getIO());
                    co_await ws.sendJson<WsLyricsMsgVO<std::string>>({
                        msgVO.musicId,
                        msgVO.type,
                        "ok"
                    });
                    break;
                }
                case WsLyricsMsgEnum::CrawlLyricsGetList: {
                    // @todo 爬取歌词并返回结果列表
                    break;
                }
                }
            }
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 测试接口: 获取到爬取的歌词
        .addEndpoint<GET>("/lyrics/test/req/{id}/to/{name}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto id = req.getPathParam(0).to<MusicDAO::PrimaryKeyType>();
                auto musicDO = musicDAO->at(id);
                std::filesystem::path assPath{std::filesystem::current_path() / "file/lyrics/ass" / (std::string{req.getPathParam(1)} + ".ass")};
                log::hxLog.info("歌词查找:", std::filesystem::current_path() / "file/music" / musicDO.path, "->", assPath);
                try {
                    toKaRaOKAssPtr->_findLyricsFromNet(
                        std::filesystem::current_path() / "file/music" / musicDO.path,
                        assPath
                    );
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
        // 测试接口: 进行日语注音
        .addEndpoint<GET>("/lyrics/test/doJp/{name}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                std::filesystem::path assPath{std::filesystem::current_path() / "file/lyrics/ass" / (std::string{req.getPathParam(0)} + ".ass")};
                log::hxLog.info("进行日语注音:", assPath);
                try {
                    toKaRaOKAssPtr->_doJapanesePhonetics(assPath);
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
        // 测试接口: 双行卡拉ok化
        .addEndpoint<GET>("/lyrics/test/toTwo/{name}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                std::filesystem::path assPath{std::filesystem::current_path() / "file/lyrics/ass" / (std::string{req.getPathParam(0)} + ".ass")};
                log::hxLog.info("双行卡拉ok化:", assPath);
                try {
                    toKaRaOKAssPtr->_toTwoLineKaraokeStyle(assPath);
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
        // 测试接口: 应用卡拉Ok模板
        .addEndpoint<GET>("/lyrics/test/call/{name}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                std::filesystem::path assPath{std::filesystem::current_path() / "file/lyrics/ass" / (std::string{req.getPathParam(0)} + ".ass")};
                log::hxLog.info("应用卡拉Ok模板:", assPath);
                try {
                    toKaRaOKAssPtr->_callApplyKaraokeTemplateLua(assPath);
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