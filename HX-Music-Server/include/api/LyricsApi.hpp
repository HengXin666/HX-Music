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
        // 批量爬取所有没有歌词的歌曲的歌词
        .addEndpoint<WS>("/lyrics/ass/karaok/all/ws", [=, scanMtx = std::make_shared<std::atomic_bool>(false)] ENDPOINT {
            struct IdAndPath {
                uint64_t id;
                std::string path;
            };
            auto ws = co_await net::WebSocketFactory::accept(req, res);
            co_await api::sendTextNoTry(ws, "任务开始: 批量爬取所有没有歌词的歌曲的歌词");
            if (scanMtx.get()->load()) {
                co_await ws.sendText("错误: 任务正在进行中, 不要重复开始!");
                co_await ws.close();
            }
            scanMtx->store(true);
            std::size_t cnt = 0;
            auto pathArr = musicDAO->lockSelect([](MusicDAO::MapType const& mp) {
                std::vector<IdAndPath> path;
                path.reserve(mp.size());
                for (auto const& [_, v] : mp) {
                    path.emplace_back(v.id, v.path);
                }
                return path;
            });
            namespace fs = std::filesystem;
            for (auto& v : pathArr) {
                fs::path assPath {
                    std::filesystem::current_path()
                    / "file/lyrics/ass"
                    / (std::to_string(v.id) + ".ass")
                };
                if (!fs::exists(assPath)) {
                    co_await api::sendTextNoTry(ws, "正在为: " + v.path + "爬取歌词");
                    {
                        container::Try<> err{};
                        do {
                            try {
                                co_await toKaRaOKAssPtr->findLyricsFromNet(
                                    std::filesystem::current_path() / "file/music" / v.path,
                                    assPath
                                ).via(req.getIO());
                                err.setVal(container::NonVoidType<>{});
                                break;
                            } catch (std::exception const& e) {
                                err.setException(std::current_exception());
                            }
                            co_await api::sendTextNoTry(ws, "发生错误: " + err.what());
                        } while (false);
                        if (!err) [[unlikely]] {
                            continue;
                        }
                    }
                    // 日语注音
                    co_await api::sendTextNoTry(ws, "正在进行日语注音...");
                    co_await toKaRaOKAssPtr->doJapanesePhonetics(
                        assPath
                    ).via(req.getIO());
                    // 双行卡拉ok化
                    co_await api::sendTextNoTry(ws, "正在双行卡拉ok化...");
                    co_await toKaRaOKAssPtr->toTwoLineKaraokeStyle(
                        assPath
                    ).via(req.getIO());
                    // 应用卡拉ok模板
                    co_await api::sendTextNoTry(ws, "正在应用卡拉ok模板...");
                    co_await toKaRaOKAssPtr->callApplyKaraokeTemplateLua(
                        assPath
                    ).via(req.getIO());
                    co_await api::sendTextNoTry(ws, v.path + "爬取歌词完毕..., 暂停等待: 5s");
                    using namespace std::chrono;
                    co_await static_cast<coroutine::EventLoop&>(req.getIO())
                        .makeTimer()
                        .sleepFor(5s);
                }
            }
            co_await api::sendTextNoTry(ws, "任务结束: 批量爬取所有没有歌词的歌曲的歌词");
            scanMtx->store(false);
            co_await ws.close();
        }, TokenInterceptor<PermissionEnum::Administrator>{})
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>