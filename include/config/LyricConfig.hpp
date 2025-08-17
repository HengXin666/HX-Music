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
#ifndef _HX_LYRIC_CONFIG_H_
#define _HX_LYRIC_CONFIG_H_

namespace HX {

/**
 * @brief 歌词配置
 */
struct LyricConfig {
    // 窗口位置和大小
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;

    // 记录全屏前的窗口大小, 其中 mae = まえ「前」
    int maeWindowX;
    int maeWindowY;
    int maeWindowWidth;
    int maeWindowHeight;

    // 偏移量
    long long lyricOffset;

    // 是否打开窗口
    bool isWindowOpened;

    // 歌词是否上锁
    bool isLocked;

    // 是否全屏
    bool isFullScreen;
};

} // namespace HX

#endif // !_HX_LYRIC_CONFIG_H_