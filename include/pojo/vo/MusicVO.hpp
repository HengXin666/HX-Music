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

#include <cstdint>
#include <string>
#include <vector>

namespace HX {

// 歌曲数据
struct MusicVO {
    uint64_t id;                        // 歌曲唯一ID
    std::string path;                   // 歌曲存放路径 (相对于 ~/file/music/)
    std::string musicName;              // 歌名
    std::vector<std::string> singers;   // 歌手
    std::string musicAlbum;             // 专辑
    uint64_t millisecondsLen;           // 毫秒长度
};

} // namespace HX