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
#ifndef _HX_MUSIC_COMMAND_H_
#define _HX_MUSIC_COMMAND_H_

#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>

/**
 * @brief 音乐相关的命令
 */
struct MusicCommand {
    /**
     * @brief 修改音量
     * @param volume 
     */
    static void setVolume(float volume) {
        GlobalSingleton::get().musicConfig.volume = volume;
        GlobalSingleton::get().music.setVolume(volume);
        SignalBusSingleton::get().volumeChanged(volume);
    }

    /**
     * @brief 音乐暂停
     */
    static void pause() {
        GlobalSingleton::get().music.pause();
        SignalBusSingleton::get().musicPaused();
    }

    /**
     * @brief 音乐播放、音乐继续
     */
    static void resume() {
        GlobalSingleton::get().music.play();
        SignalBusSingleton::get().musicResumed();
    }
};

#endif // !_HX_MUSIC_COMMAND_H_