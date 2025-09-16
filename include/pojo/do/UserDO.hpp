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

#include <db/SQLiteMeta.hpp>

namespace HX {

// 要求 越大的权限, 值越小
enum class PermissionEnum : uint8_t {
    Administrator,  // 管理员, 可以新增用户
    RegularUser,    // 普通用户, 可以创建歌单、上传音乐、编辑自己创建的歌单等
    ReadOnlyUser,   // 只读用户
};

/**
 * @brief 用户数据
 */
struct UserDO {
    db::PrimaryKey<uint64_t> id;            // 唯一Id
    std::string name;                       // 用户名 (依旧唯一)
    std::string signature;                  // 个性签名
    std::string salt;                       // 密码盐
    std::string password;                   // 密码 (加密加盐)
    std::vector<uint64_t> createdPlaylist;  // 创建的歌单
    std::vector<uint64_t> savedPlaylist;    // 收藏的歌单
    PermissionEnum permissionLevel;         // 权限分级
};

} // namespace HX