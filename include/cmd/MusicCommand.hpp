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
    static void switchMusic(HX::PlayQueue::iterator it) {
        GlobalSingleton::get().playQueue.setNowIt(it);
        SignalBusSingleton::get().newSongLoaded(
            HX::MusicInfo{QFileInfo{
                it->getData()
            }}
        );
    }

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
        GlobalSingleton::get().musicConfig.isPlay = false;
        GlobalSingleton::get().music.pause();
        SignalBusSingleton::get().musicPaused();
    }

    /**
     * @brief 音乐播放、音乐继续
     */
    static void resume() {
        GlobalSingleton::get().musicConfig.isPlay = true;
        GlobalSingleton::get().music.play();
        SignalBusSingleton::get().musicResumed();
    }

    /**
     * @brief 修改播放模式
     */
    static void setPlayMode(PlayMode mode) {
        GlobalSingleton::get().musicConfig.playMode = mode;
        SignalBusSingleton::get().playModeChanged(mode);
    }

    /**
     * @brief 下一首
     */
    static void nextMusic() {
        if (auto it = GlobalSingleton::get().playQueue.next()) {
            switchMusic(*it);
            SignalBusSingleton::get().musicResumed();
        }
    }

    /**
     * @brief 上一首
     */
     static void prevMusic() {
        if (auto it = GlobalSingleton::get().playQueue.prev()) {
            switchMusic(*it);
            SignalBusSingleton::get().musicResumed();
        }
    }
};

#endif // !_HX_MUSIC_COMMAND_H_