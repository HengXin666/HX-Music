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
#ifndef _HX_FILE_INFO_H_
#define _HX_FILE_INFO_H_

#include <QString>
#include <QStringList>

namespace HX {

struct FileInfo {

/**
 * @brief 转换`字节`为人类可读的单位
 * @param bytes 
 * @return QString 
 */
inline static QString convertByteSizeToHumanReadable(float bytes) {
    QStringList unitList {
        "KiB",
        "MiB",
        "GiB",
        "TiB",
    };

    QStringListIterator it{unitList};
    QString unit{"B"};

    while(bytes >= 1024.0 && it.hasNext())
     {
        unit = it.next();
        bytes /= 1024.0;
    }
    return QString().setNum(bytes,'f',2) + " " + unit;
}

};

} // namespace HX

#endif // !_HX_FILE_INFO_H_