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
            auto listDO = api::toDO<MusicListDO>(api::getVO<MusicListVO>(req));
            auto const& newDO = DAOSingleton::get().musicListDAO.add(listDO);
            co_await res.setStatusAndContent(Status::CODE_200, log::toString(newDO.id))
                        .sendRes();
        })
        // 编辑歌单
        .addEndpoint<POST>("/musicList/update", [] ENDPOINT {
            auto vo = api::getVO<MusicListVO>(req);
            co_return ;
        })
        // 删除歌单
        .addEndpoint<POST, DEL>("/musicList/del/{id}", [] ENDPOINT {
            co_return ;
        })
        // 获取歌单
        .addEndpoint<GET>("/musicList/select/{id}", [] ENDPOINT {
            co_return ;
        })
        // 获取全部歌单
        .addEndpoint<GET>("/musicList/selectAll", [] ENDPOINT {
            co_return ;
        })
        // 为歌单添加歌曲
        .addEndpoint<POST>("/musicList/{id}/addMusic/{musicId}", [] ENDPOINT {
            co_return ;
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