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

#include <pojo/do/MusicDO.hpp>

namespace HX {

struct MusicListDO {
    db::PrimaryKey<uint64_t> id;            // 歌单id (唯一), 定义本地歌单为默认, 为 `localPlaylist`
    std::string description;                // 歌单描述
    std::vector<uint64_t> songList;         // 歌曲列表
};

} // namespace HX