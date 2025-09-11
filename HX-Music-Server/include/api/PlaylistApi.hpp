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

#include <pojo/vo/JsonVO.hpp>
#include <pojo/vo/PlaylistInfoVO.hpp>
#include <pojo/vo/PlaylistInfoListVO.hpp>
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
    if (!playlistDAO) {
        throw std::runtime_error{"wdf? this is nullptr"};
    }
    HX_ENDPOINT_BEGIN
        // 创建歌单
        .addEndpoint<POST>("/playlist/make", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto vo = co_await api::getVO<PlaylistInfoVO>(req);
                uint64_t id = playlistDAO->add<PlaylistDO>({
                    {},
                    std::move(vo.name),
                    std::move(vo.description),
                    {}
                }).id;
                co_await api::setJsonSucceed(id, res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("创建歌单失败", res).sendRes();
            });
        })
        // 编辑歌单
        .addEndpoint<POST>("/playlist/update", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                // @todo
                playlistDAO->update(
                    api::toDO<PlaylistDO>(co_await api::getVO<PlaylistVO>(req))
                );
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("编辑失败", res).sendRes();
            });
        })
        // 删除歌单
        .addEndpoint<POST, DEL>("/playlist/del/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                playlistDAO->del(id);
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("删除失败", res).sendRes();
            });
        })
        // 获取歌单
        .addEndpoint<GET>("/playlist/select/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                auto const& listDO = playlistDAO->at(id);
                co_await api::setJsonSucceed<PlaylistVO>({
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
                                musicDO.musicAlbum,
                                musicDO.millisecondsLen
                            );
                        }
                        return songList;
                    }()
                }, res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("获取歌单失败", res).sendRes();
            });
        })
        // 获取全部歌单
        .addEndpoint<GET>("/playlist/selectAll", [=] ENDPOINT {
            co_await api::setJsonSucceed(
                playlistDAO->lockSelect([](PlaylistDAO::MapType const& mp) noexcept {
                PlaylistInfoListVO res;
                for (auto const& [id, val] : mp) {
                    res.infoList.emplace_back(id, val.name, val.description, val.songIdList.size());
                }
                return res;
            }), res).sendRes();
        })
        // 获取歌单简介
        .addEndpoint<GET>("/playlist/info/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                co_await api::setJsonSucceed([&]() -> PlaylistInfoVO {
                    auto const& listDO = playlistDAO->at(id);
                    return {
                        id,
                        listDO.name,
                        listDO.description,
                        listDO.songIdList.size()
                    };
                }(), res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("获取歌单简介失败", res).sendRes();
            });
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
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌单添加歌曲失败", res).sendRes();
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