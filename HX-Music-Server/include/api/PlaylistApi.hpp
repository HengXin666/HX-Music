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
#include <pojo/vo/JsonVO.hpp>
#include <pojo/vo/PlaylistInfoVO.hpp>
#include <pojo/vo/PlaylistInfoListVO.hpp>
#include <pojo/vo/PlaylistVO.hpp>
#include <pojo/vo/IdListVO.hpp>
#include <interceptor/TokenInterceptor.hpp>
#include <dao/MusicDAO.hpp>
#include <dao/PlaylistDAO.hpp>
#include <dao/UserDAO.hpp>

#include <api/ApiMacro.hpp>

namespace HX {

/**
 * @brief 音乐列表 相关服务 API
 */
HX_SERVER_API_BEGIN(PlaylistApi) {
    auto musicDAO 
        = dao::MemoryDAOPool::get<MusicDAO, config::MusicDbPath>();
    auto playlistDAO 
        = dao::MemoryDAOPool::get<PlaylistDAO, config::PlaylistDbPath>();
    auto userDAO
        = dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>();
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
                auto userId = getTokenData(req).userId;
                auto createdPlaylist
                    = userDAO->at(userId, &UserDO::createdPlaylist);
                createdPlaylist.emplace_back(id);
                userDAO->updateBy(userId, db::FieldPair{
                    &UserDO::createdPlaylist,
                    createdPlaylist
                });
                co_await api::setJsonSucceed(id, res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("创建歌单失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
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
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 删除歌单
        .addEndpoint<POST, DEL>("/playlist/del/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto id = req.getPathParam(0).to<uint64_t>();
                playlistDAO->del(id);
                auto userId = getTokenData(req).userId;
                auto createdPlaylist
                    = userDAO->at(userId, &UserDO::createdPlaylist);
                createdPlaylist.erase(std::ranges::find(createdPlaylist, id));
                userDAO->updateBy(userId, db::FieldPair{
                    &UserDO::createdPlaylist,
                    createdPlaylist
                });
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("删除失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
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
                            auto musicDO = musicDAO->at(id);
                            songList.emplace_back(
                                musicDO.id,
                                std::move(musicDO.path),
                                std::move(musicDO.musicName),
                                std::move(musicDO.singers),
                                std::move(musicDO.musicAlbum),
                                musicDO.millisecondsLen
                            );
                        }
                        return songList;
                    }()
                }, res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("获取歌单失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
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
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
        // 获取用户创建的歌单
        .addEndpoint<GET>("/playlist/selectAll/created", [=] ENDPOINT {
            auto createdList = userDAO->at(
                getTokenData(req).userId, &UserDO::createdPlaylist
            );
            co_await api::setJsonSucceed(
                playlistDAO->lockSelect([&](PlaylistDAO::MapType const& mp) noexcept {
                PlaylistInfoListVO res;
                for (auto id : createdList) {
                    auto const& val = mp.at(id);
                    res.infoList.emplace_back(id, val.name, val.description, val.songIdList.size());
                }
                return res;
            }), res).sendRes();
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
        // 获取用户保存的歌单
        .addEndpoint<GET>("/playlist/selectAll/saved", [=] ENDPOINT {
            auto savedPlaylist = userDAO->at(
                getTokenData(req).userId, &UserDO::savedPlaylist
            );
            co_await api::setJsonSucceed(
                playlistDAO->lockSelect([&](PlaylistDAO::MapType const& mp) noexcept {
                PlaylistInfoListVO res;
                for (auto id : savedPlaylist) {
                    auto const& val = mp.at(id);
                    res.infoList.emplace_back(id, val.name, val.description, val.songIdList.size());
                }
                return res;
            }), res).sendRes();
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
        // 获取歌单简介
        .addEndpoint<GET>("/playlist/info/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                co_await api::setJsonSucceed([&]() -> PlaylistInfoVO {
                    auto listDO = playlistDAO->at(id);
                    return {
                        id,
                        std::move(listDO.name),
                        std::move(listDO.description),
                        listDO.songIdList.size() // @todo 性能
                    };
                }(), res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("获取歌单简介失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
        // 为歌单添加歌曲
        .addEndpoint<POST>("/playlist/{id}/addMusic/{musicId}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto id = req.getPathParam(0).to<uint64_t>();
                if (userDAO->lockSelect([&](UserDAO::MapType const& mp) {
                    auto const& arr
                        = mp.at(getTokenData(req).userId).createdPlaylist;
                    return std::ranges::find(arr, id) == arr.end();
                })) {
                    co_return co_await api::setJsonError("只能修改创建的歌单", res).sendRes();
                }
                auto musicId = req.getPathParam(1).to<uint64_t>();
                auto listDO = playlistDAO->at(id);
                if (std::ranges::find(listDO.songIdList, musicId) != listDO.songIdList.end()) {
                    co_return co_await api::setJsonError("添加失败: 音乐已存在", res).sendRes();
                }
                listDO.songIdList.push_back(musicId);
                playlistDAO->update(std::move(listDO));
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌单添加歌曲失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 为歌单删除歌曲
        .addEndpoint<POST, DEL>("/playlist/{id}/delMusic/{idx}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto id = req.getPathParam(0).to<uint64_t>(),
                     idx = req.getPathParam(1).to<uint64_t>();
                auto playlistDO = playlistDAO->at(id);
                if (idx >= playlistDO.songIdList.size()) [[unlikely]] {
                    co_return co_await api::setJsonError("索引越界", res).sendRes();;
                }
                playlistDO.songIdList.erase(playlistDO.songIdList.begin() + idx);
                playlistDAO->update(std::move(playlistDO));
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌曲删除失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 完整更新歌单歌曲顺序
        .addEndpoint<POST>("/playlist/updateMusicOrder/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto listVO = co_await api::getVO<IdListVO>(req);
                auto id = req.getPathParam(0).to<uint64_t>();
                auto songIdList = playlistDAO->at(id, &PlaylistDO::songIdList);
                if (songIdList.size() != listVO.idList.size()) [[unlikely]] {
                    co_return co_await api::setJsonError("数据不一致, 请刷新", res).sendRes();
                }
                playlistDAO->updateBy(id, db::FieldPair{&PlaylistDO::songIdList, listVO.idList});
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("调整歌曲位置失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 完整更新歌单顺序 (用户创建歌单)
        .addEndpoint<POST>("/playlist/updateOrder/created", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto listVO = co_await api::getVO<IdListVO>(req);
                auto id = getTokenData(req).userId;
                auto songIdList = userDAO->at(id, &UserDO::createdPlaylist);
                if (songIdList.size() != listVO.idList.size()) [[unlikely]] {
                    co_return co_await api::setJsonError("数据不一致, 请刷新", res).sendRes();
                }
                userDAO->updateBy(id, db::FieldPair{&UserDO::createdPlaylist, listVO.idList});
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("调整歌单位置失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 完整更新歌单顺序 (用户创建歌单)
        .addEndpoint<POST>("/playlist/updateOrder/saved", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto listVO = co_await api::getVO<IdListVO>(req);
                auto id = getTokenData(req).userId;
                auto songIdList = userDAO->at(id, &UserDO::savedPlaylist);
                if (songIdList.size() != listVO.idList.size()) [[unlikely]] {
                    co_return co_await api::setJsonError("数据不一致, 请刷新", res).sendRes();
                }
                userDAO->updateBy(id, db::FieldPair{&UserDO::savedPlaylist, listVO.idList});
                co_await api::setJsonSucceed<std::string>("ok", res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("调整歌单位置失败", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>