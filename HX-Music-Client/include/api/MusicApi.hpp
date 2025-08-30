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
#include <pojo/SongInformation.hpp>

#include <api/Api.hpp>

#include <pojo/vo/MusicVO.hpp>

namespace HX {

/**
 * @brief 客户端 歌曲相关请求 API
 */
struct MusicApi {
    static container::FutureResult<SongInformation> selectById(uint64_t id) {
        return NetSingleton::get().getReq("/music/info/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) -> SongInformation {
                if (!t) [[unlikely]] {
                    t.rethrow();
                }
                auto res = t.move();
                auto jsonVO = api::getVO<vo::JsonVO<MusicVO>>(res);
                if (jsonVO.isError()) [[unlikely]] {
                    throw std::runtime_error{std::move(jsonVO.msg)};
                }
                return *std::move(jsonVO.data);
            }
        );
    }
};

} // namespace HX