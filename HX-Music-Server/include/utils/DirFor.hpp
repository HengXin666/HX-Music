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

#include <filesystem>
#include <vector>

namespace HX::utils {

/**
 * @brief 递归遍历文件夹
 * @tparam Lambda 
 * @param root 根文件夹
 * @param ignoreDirs 忽略的文件夹
 * @param callback 回调函数, 传入 const fs::path& relativePath
 */
template <typename Lambda>
void traverseDirectory(
    const std::filesystem::path& root,
    const std::vector<std::string>& ignoreDirs,
    Lambda&& callback
) {
    namespace fs = std::filesystem;
    if (!fs::exists(root)) {
        return;
    }

    std::error_code ec;
    fs::recursive_directory_iterator it(
        root, fs::directory_options::skip_permission_denied, ec);
    fs::recursive_directory_iterator end;

    const auto rootLen = root.string().size();

    for (; it != end; it.increment(ec)) {
        if (ec) {
            ec.clear();
            continue;
        }

        const auto& entry = *it;
        const fs::path& currentPath = entry.path();

        // 计算相对路径 (避免 lexically_relative 的高成本)
        std::string relative = currentPath.string().substr(rootLen);
        if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\')) {
            relative.erase(0, 1);
        }

        // 检查是否忽略目录
        if (entry.is_directory(ec)) {
            bool skip = false;
            for (const auto& ignore : ignoreDirs) {
                if (currentPath.filename() == ignore) {
                    it.disable_recursion_pending();
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;
        }

        // 回调
        callback(relative);
    }
}

} // namespace HX::utils