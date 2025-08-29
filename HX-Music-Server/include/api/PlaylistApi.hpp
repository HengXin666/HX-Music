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

#include <pojo/vo/PlaylistVO.hpp>
#include <dao/MusicDAO.hpp>
#include <dao/PlaylistDAO.hpp>

namespace HX {

/**
 * @brief 音乐列表 相关服务 API
 */
HX_SERVER_API_BEGIN(PlaylistApi) {
    auto musicDAO 
        = dao::MemoryDAOPool::get<MusicDAO, "./file/db/music.db">();
    auto playlistDAO 
        = dao::MemoryDAOPool::get<PlaylistDAO, "./file/db/playlist.db">();
    HX_ENDPOINT_BEGIN
        // 创建歌单
        .addEndpoint<POST>("/playlist/make", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto listDO = api::toDO<PlaylistDO>(api::getVO<PlaylistVO>(req));
                auto const& newDO = playlistDAO->add(listDO);
                co_await res.setStatusAndContent(Status::CODE_200, log::toString(newDO.id))
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "创建失败")
                            .sendRes();
            });
        })
        // 编辑歌单
        .addEndpoint<POST>("/playlist/update", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto const& newDO = playlistDAO->update(
                    api::toDO<PlaylistDO>(api::getVO<PlaylistVO>(req))
                );
                co_await res.setStatusAndContent(Status::CODE_200, log::toString(newDO.id))
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "编辑失败")
                            .sendRes();
            });
        })
        // 删除歌单
        .addEndpoint<POST, DEL>("/playlist/del/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                playlistDAO->del(id);
                co_await res.setStatusAndContent(Status::CODE_200, "ok")
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "删除失败")
                            .sendRes();
            });
        })
        // 获取歌单
        .addEndpoint<GET>("/playlist/select/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                auto const& listDO = playlistDAO->at(id);
                PlaylistVO vo {
                    listDO.id,
                    listDO.name,
                    listDO.description,
                    [&] {
                        std::vector<MusicVO> songList;
                        for (auto const id : listDO.songIdList) {
                            auto& musicDO = musicDAO->at(id);
                            songList.emplace_back(
                                musicDO.id,
                                musicDO.path,
                                musicDO.musicName,
                                musicDO.singers,
                                musicDO.musicAlbum
                            );
                        }
                        return songList;
                    }()
                };
                api::setVO(vo, res);
                co_await res.setResLine(Status::CODE_200)
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "获取歌单失败")
                            .sendRes();
            });
        })
        // 获取全部歌单
        .addEndpoint<GET>("/playlist/selectAll", [] ENDPOINT {
            co_return ;
        })
        // 为歌单添加歌曲
        .addEndpoint<POST>("/playlist/{id}/addMusic/{musicId}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id, musicId;
                reflection::fromJson(id, req.getPathParam(0));
                reflection::fromJson(musicId, req.getPathParam(1));
                auto listDO = playlistDAO->at(id);
                listDO.songIdList.push_back(musicId);
                playlistDAO->update(std::move(listDO));
                co_await res.setStatusAndContent(Status::CODE_200, "ok")
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "歌单添加歌曲失败")
                            .sendRes();
            });
        })
        // 为歌单删除歌曲
        .addEndpoint<POST, DEL>("/playlist/{id}/delMusic/{musicId}", [] ENDPOINT {
            co_return ;
        })
        // 为歌单交换歌曲位置
        .addEndpoint<POST>("/playlist/{id}/swapMusic", [] ENDPOINT {
            co_return ;
        })
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>