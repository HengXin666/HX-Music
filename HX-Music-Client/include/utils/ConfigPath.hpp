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

#include <QStandardPaths>
#include <QString>
#include <QDir>

namespace HX {

/**
 * @brief 获取配置文件路径
 * @param fileName 文件名
 * @return string 
 */
inline std::string getConfigPath(QString const& fileName) noexcept {
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir{}.mkpath(dirPath); // 确保目录存在
    return (dirPath + '/' + fileName).toStdString();
}

} // namespace HX