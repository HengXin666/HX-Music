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
#ifndef _HX_MUSIC_CONFIG_H_
#define _HX_MUSIC_CONFIG_H_

#include <QString>

/**
 * @brief 播放模式
 */
enum class PlayMode {
    ListLoop,   // 列表循环
    RandomPlay, // 随机播放
    SinglePlay, // 单曲播放
    SingleLoop, // 单曲循环
};

/**
 * @brief 音乐配置
 */
struct MusicConfig {
    float volume;       // 音量大小
    PlayMode playMode;  // 播放模式
    qint64 position;    // 播放位置
};

#endif // !_HX_MUSIC_CONFIG_H_