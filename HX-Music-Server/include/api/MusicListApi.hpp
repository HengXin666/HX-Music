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

#include <pojo/vo/MusicListVO.hpp>
#include <singleton/DAOSingleton.hpp>

namespace HX {

/**
 * @brief 音乐列表 相关服务 API
 */
HX_ServerApiBegin(MusicListApi) {
    HX_EndpointBegin
        // 创建歌单
        .addEndpoint<POST>("/musicList/make", [] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto listDO = api::toDO<MusicListDO>(api::getVO<MusicListVO>(req));
                auto const& newDO = DAOSingleton::get().musicListDAO.add(listDO);
                co_await res.setStatusAndContent(Status::CODE_200, log::toString(newDO.id))
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "创建失败")
                            .sendRes();
            });
        })
        // 编辑歌单
        .addEndpoint<POST>("/musicList/update", [] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto const& newDO = DAOSingleton::get().musicListDAO.update(
                    api::toDO<MusicListDO>(api::getVO<MusicListVO>(req))
                );
                co_await res.setStatusAndContent(Status::CODE_200, log::toString(newDO.id))
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "编辑失败")
                            .sendRes();
            });
        })
        // 删除歌单
        .addEndpoint<POST, DEL>("/musicList/del/{id}", [] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                DAOSingleton::get().musicListDAO.del(id);
                co_await res.setStatusAndContent(Status::CODE_200, "ok")
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "删除失败")
                            .sendRes();
            });
        })
        // 获取歌单
        .addEndpoint<GET>("/musicList/select/{id}", [] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id;
                reflection::fromJson(id, req.getPathParam(0));
                auto const& listDO = DAOSingleton::get().musicListDAO.at(id);
                MusicListVO vo {
                    listDO.id,
                    listDO.name,
                    listDO.description,
                    listDO.songList
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
        .addEndpoint<GET>("/musicList/selectAll", [] ENDPOINT {
            co_return ;
        })
        // 为歌单添加歌曲
        .addEndpoint<POST>("/musicList/{id}/addMusic/{musicId}", [] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                uint64_t id, musicId;
                reflection::fromJson(id, req.getPathParam(0));
                reflection::fromJson(musicId, req.getPathParam(1));
                auto listDO = DAOSingleton::get().musicListDAO.at(id);
                listDO.songList.push_back(musicId);
                DAOSingleton::get().musicListDAO.update(std::move(listDO));
                co_await res.setStatusAndContent(Status::CODE_200, "ok")
                            .sendRes();
            }, [&] CO_FUNC {
                co_await res.setStatusAndContent(Status::CODE_500, "歌单添加歌曲失败")
                            .sendRes();
            });
        })
        // 为歌单删除歌曲
        .addEndpoint<POST, DEL>("/musicList/{id}/delMusic/{musicId}", [] ENDPOINT {
            co_return ;
        })
        // 为歌单交换歌曲位置
        .addEndpoint<POST>("/musicList/{id}/swapMusic", [] ENDPOINT {
            co_return ;
        })
    HX_EndpointEnd;
} HX_ServerApiEnd;

} // namespace HX

#include <api/UnApiMacro.hpp>