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

#include <pojo/do/MusicListDO.hpp>

namespace HX {

// 歌单数据
struct MusicListVO {
    uint64_t id;                            // 歌单ID
    std::string name;                       // 歌单名称
    std::string description;                // 歌单描述
    std::vector<uint64_t> songList;         // 歌曲列表

    operator MusicListDO() && noexcept {
        return MusicListDO {
            id,
            std::move(name),
            std::move(description),
            std::move(songList)
        };
    }
};

} // namespace HX