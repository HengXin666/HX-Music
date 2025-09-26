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

#include <api/Api.hpp>
#include <pojo/vo/WsLyricsMsgVO.hpp>

namespace HX {

/**
 * @brief 歌词相关 API
 */
struct LyricsApi {
    /**
     * @brief 获取 ASS 歌词
     * @param id 歌曲id
     * @return container::FutureResult<std::string> 
     */
    static container::FutureResult<std::string> getAssLyrics(uint64_t id) {
        return NetSingleton::get().getReq("/lyrics/ass/select/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) {
                if (!t) [[unlikely]] {
                    t.rethrow();
                } else if (t.get().status / 100 != 2) [[unlikely]] {
                    api::throwVoMsg(t.move());
                }
                return t.move().body;
            });
    }

    /**
     * @brief 爬取 卡拉OK Ass 歌词
     * @param id 歌曲id
     * @return container::FutureResult<> 
     */
    static container::FutureResult<container::Try<>> crawlKaRaOKAssLyricsByWs(uint64_t id) {
        return NetSingleton::get().wsReq(
            "/lyrics/ass/karaok/ws",
            [id](net::WebSocketClient ws) -> coroutine::Task<> {
                co_await ws.sendJson<WsLyricsMsgVO<>>({
                    id,
                    WsLyricsMsgEnum::CrawlLyrics
                });
                log::hxLog.debug("正在爬取歌词");
                if (auto msg = co_await ws.recvJson<WsLyricsMsgVO<std::string>>();
                    msg.data.value() != "ok"
                ) {
                    throw std::runtime_error{msg.data.value()};
                }
                co_await ws.sendJson<WsLyricsMsgVO<>>({
                    id,
                    WsLyricsMsgEnum::JpTranscription
                });
                log::hxLog.debug("正在进行日语注音");
                if (auto msg = co_await ws.recvJson<WsLyricsMsgVO<std::string>>();
                    msg.data.value() != "ok"
                ) {
                    throw std::runtime_error{msg.data.value()};
                }
                co_await ws.sendJson<WsLyricsMsgVO<>>({
                    id,
                    WsLyricsMsgEnum::TwoLineKaraokeStyle
                });
                log::hxLog.debug("正在转为双行卡拉ok");
                if (auto msg = co_await ws.recvJson<WsLyricsMsgVO<std::string>>();
                    msg.data.value() != "ok"
                ) {
                    throw std::runtime_error{msg.data.value()};
                }
                co_await ws.sendJson<WsLyricsMsgVO<>>({
                    id,
                    WsLyricsMsgEnum::CallKaraokeTemplateLua
                });
                log::hxLog.debug("正在应用卡拉ok模板");
                if (auto msg = co_await ws.recvJson<WsLyricsMsgVO<std::string>>();
                    msg.data.value() != "ok"
                ) {
                    throw std::runtime_error{msg.data.value()};
                }
                log::hxLog.debug("爬取歌词: [", id, "] 完成!");
            }
        );
    }

    template <typename Cb>
        requires (requires (Cb&& cb) {
            { cb(std::string{}) };
        })
    static container::FutureResult<container::Try<>> startScan(Cb&& cb) {
        return NetSingleton::get().wsReq(
            "/lyrics/ass/karaok/all/ws",
            [_cb = std::forward<Cb>(cb)](net::WebSocketClient ws) -> coroutine::Task<> {
                for (;;) {
                    _cb(co_await ws.recvText());
                }
            }
        );
    }
};

} // namespace HX