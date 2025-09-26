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
#include <vector>
#include <string>

#include <pojo/do/UserDO.hpp>

namespace HX {

struct UserInfoVO {
    uint64_t userId;                        // 用户id
    std::string name;                       // 用户名
    std::size_t createdPlaylistLen;         // 用户创建的歌单个数
    std::size_t savedPlaylistLen;           // 用户收藏的歌单个数
    PermissionEnum permissionLevel;         // 权限分级
};

/**
 * @brief 用户简述数据列表 JsonVO
 */
struct UserInfoListVO {
    std::vector<UserInfoVO> list;
};

} // namespace HX