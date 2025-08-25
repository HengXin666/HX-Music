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

namespace HX {

/**
 * @brief 音乐服务 API
 */
HX_ServerApiBegin(MusicApi) {
    HX_EndpointBegin
        .addEndpoint<GET, HEAD>("/music/download/**", [] ENDPOINT {
            using namespace std::string_literals;
            bool isErr = false;
            try {
                co_await res.useRangeTransferFile(
                     req.getRangeRequestView(),
                     "./file/music/"s += req.getUniversalWildcardPath()
                );
            } catch (...) {
                isErr = true;
            }
            if (isErr) [[unlikely]] {
                co_await res.setStatusAndContent(Status::CODE_500, "文件不存在!")
                            .sendRes();
            }
            co_return;
        })
        .addEndpoint<GET>("/", [] ENDPOINT {
            co_await res.setStatusAndContent(Status::CODE_200, "Hi! HX-Music-Server!")
                        .sendRes();
        })
    HX_EndpointEnd;
} HX_ServerApiEnd;

} // namespace HX

#include <api/UnApiMacro.hpp>