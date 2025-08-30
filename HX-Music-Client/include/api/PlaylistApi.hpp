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

#include <pojo/Playlist.hpp>
#include <singleton/NetSingleton.hpp>

#include <api/Api.hpp>
#include <pojo/vo/PlaylistVO.hpp>

namespace HX {

/**
 * @brief 客户端 歌单相关请求 API
 */
struct PlaylistApi {
    /**
     * @brief 获取歌单
     * @param id 
     * @return auto 
     */
    static auto selectById(uint64_t id) {
        return NetSingleton::get().getReq("/playlist/select/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) -> Playlist {
            if (!t) [[unlikely]] {
                log::hxLog.error("selectById:", t.what());
                t.rethrow();
            }
            log::hxLog.info("selectById:", t.get());
            auto res = t.move();
            auto jsonVo = api::getVO<vo::JsonVO<PlaylistVO>>(res);
            if (jsonVo.code == vo::VOCode::Err) [[unlikely]] {
                throw std::runtime_error{std::move(jsonVo.msg)};
            }
            auto playlistVO = *jsonVo.data;
            return {
                playlistVO.id,
                playlistVO.name,
                playlistVO.description,
                [&] {
                    std::vector<SongInformation> songList;
                    for (auto&& musicVO : playlistVO.songList) {
                        songList.emplace_back(
                            musicVO.id,
                            std::move(musicVO.path),
                            std::move(musicVO.musicName),
                            std::move(musicVO.singers),
                            std::move(musicVO.musicAlbum)
                        );
                    }
                    return songList;
                }()
            };
        });
    }
};

} // namespace HX