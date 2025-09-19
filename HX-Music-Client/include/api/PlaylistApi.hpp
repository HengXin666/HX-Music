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
#include <pojo/PlaylistInfo.hpp>

#include <HXLibs/utils/NumericBaseConverter.hpp>

#include <api/Api.hpp>
#include <pojo/vo/PlaylistInfoVO.hpp>
#include <pojo/vo/PlaylistInfoListVO.hpp>
#include <pojo/vo/PlaylistVO.hpp>
#include <pojo/vo/SongIdListVO.hpp>

namespace HX {

/**
 * @brief 客户端 歌单相关请求 API
 */
struct PlaylistApi {
    /**
     * @brief 创建歌单
     * @param playlistInfo 歌单信息
     * @return container::FutureResult<uint64_t> 新歌单的id
     */
    static container::FutureResult<uint64_t> makePlaylist(PlaylistInfoVO&& playlistInfo) {
        return NetSingleton::get().postReq("/playlist/make", std::move(playlistInfo))
            .thenTry([](container::Try<net::ResponseData> t) {
                return api::checkTryAndStatusAndJsonVO<uint64_t>(std::move(t));
            });
    }

    /**
     * @brief 获取歌单
     * @param id 
     * @return container::FutureResult<Playlist> 
     */
    static container::FutureResult<Playlist> selectById(uint64_t id) {
        return NetSingleton::get().getReq("/playlist/select/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) -> Playlist {
                if (!t) [[unlikely]] {
                    t.rethrow();
                }
                auto res = t.move();
                auto jsonVo = api::getVO<vo::JsonVO<PlaylistVO>>(res);
                if (jsonVo.isError()) [[unlikely]] {
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
                                std::move(musicVO.musicAlbum),
                                musicVO.millisecondsLen
                            );
                        }
                        return songList;
                    }()
                };
            }
        );
    }

    /**
     * @brief 获取所有歌单简介列表
     * @return container::FutureResult<std::vector<PlaylistInfo>> 
     */
    static container::FutureResult<std::vector<PlaylistInfo>> selectAllPlaylist() {
        return NetSingleton::get().getReq("/playlist/selectAll")
            .thenTry([](container::Try<net::ResponseData> t) {
                return api::checkTryAndStatusAndJsonVO<PlaylistInfoListVO>(std::move(t)).infoList;
            });
    }

    /**
     * @brief 获取所有用户创建的歌单简介列表
     * @return container::FutureResult<std::vector<PlaylistInfo>> 
     */
    static container::FutureResult<std::vector<PlaylistInfo>> selectUserCreatedAllPlaylist() {
        return NetSingleton::get().getReq("/playlist/selectAll/created")
            .thenTry([](container::Try<net::ResponseData> t) {
                return api::checkTryAndStatusAndJsonVO<PlaylistInfoListVO>(std::move(t)).infoList;
            });
    }

    /**
     * @brief 获取所有用户保存的歌单简介列表
     * @return container::FutureResult<std::vector<PlaylistInfo>> 
     */
    static container::FutureResult<std::vector<PlaylistInfo>> selectUserSavedAllPlaylist() {
        return NetSingleton::get().getReq("/playlist/selectAll/saved")
            .thenTry([](container::Try<net::ResponseData> t) {
                return api::checkTryAndStatusAndJsonVO<PlaylistInfoListVO>(std::move(t)).infoList;
            });
    }

    /**
     * @brief 获取歌单简介
     * @param id 
     * @return container::FutureResult<PlaylistInfo> 
     */
    static container::FutureResult<PlaylistInfo> getPlaylistInfo(uint64_t id) {
        return NetSingleton::get().getReq("/playlist/info/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) {
                return api::checkTryAndStatusAndJsonVO<PlaylistInfo>(std::move(t));
            });
    }

    /**
     * @brief 删除歌单
     * @param id 
     * @return container::FutureResult<> 
     */
    static container::FutureResult<> delPlaylist(uint64_t id) {
        return NetSingleton::get().delReq("/playlist/del/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) {
                api::checkTryAndStatusAndJsonVO<std::string>(std::move(t));
            });
    }

    /**
     * @brief 为歌单添加歌曲
     * @param playlistId 歌单ID
     * @param musicId 歌曲ID
     * @return container::FutureResult<> 
     */
    static container::FutureResult<> addMusic(uint64_t playlistId, uint64_t musicId) {
        return NetSingleton::get().postReq("/playlist/"
            + std::to_string(playlistId)
            + "/addMusic/"
            + std::to_string(musicId),
            {},
            net::HttpContentType::None
        ).thenTry([](container::Try<net::ResponseData> t) {
            api::checkTryAndStatusAndJsonVO<std::string>(std::move(t));
        });
    }

    /**
     * @brief 交换歌单的歌曲
     * @param playlistId 歌单id
     * @param from 来源
     * @param to 目的地
     * @return container::FutureResult<container::Try<>> 
     */
    static container::FutureResult<> insertMusic(
        uint64_t playlistId, std::vector<uint64_t> songIdList
    ) {
        return NetSingleton::get().postReq(
            "/playlist/updateOrder/" + std::to_string(playlistId),
            SongIdListVO{std::move(songIdList)}
        ).thenTry([](container::Try<net::ResponseData> t) {
            api::checkTryAndStatusAndJsonVO<std::string>(std::move(t));
        });
    }
};

} // namespace HX