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

namespace HX {

/**
 * @brief 音乐相关的命令
 */
struct MusicCommand {
    /**
     * @brief 选择音乐(双击), 此时是已知需要播放的音乐的, 因此会添加到播放队列
     * @param it 
     */
    static void selectMusic(PlayQueue::Type it) {
        GlobalSingleton::get().playQueue.push(it);
        switchMusic(it);
    }

    /**
     * @brief 切换音乐, 并且播放
     * @param it 
     */
    static void switchMusic(PlayQueue::Type it) {
        GlobalSingleton::get().playQueue.push(it);
        auto fileInfo = QFileInfo{it};
        auto info = MusicInfo{fileInfo};
        SignalBusSingleton::get().newSongLoaded(info);
        GlobalSingleton::get().music.setLengthInMilliseconds(info.getLengthInMilliseconds());
        GlobalSingleton::get().music.switchMusic(info.filePath()).play();
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
        if (GlobalSingleton::get().playQueue.empty())
            return;
        GlobalSingleton::get().musicConfig.isPlay = true;
        // 不知道为什么, 暂停一段时间后, 重新播放会没有声音...
        // 但是跳转后又有... 那我只好给你手动跳转一下了
        // 难道是因为qt层次没有动. 而ff层没有停吗? 这不对吧
        setMusicPos(GlobalSingleton::get().music.getNowPos());
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
        switch (GlobalSingleton::get().musicConfig.playMode) {
        case PlayMode::RandomPlay:  // 随机播放
            if (auto it = GlobalSingleton::get().playQueue.next()) {
                switchMusic(*it);
                SignalBusSingleton::get().musicResumed();
                break;
            }
        [[fallthrough]];
        case PlayMode::ListLoop:    // 列表循环
        case PlayMode::SinglePlay:  // 单曲播放
        case PlayMode::SingleLoop:  // 单曲循环
            // 从 SongListModel 中得
            break;
        case PlayMode::PlayModeCnt: // !保留!
            break;
        }
    }

    /**
     * @brief 上一首
     */
    static void prevMusic() {
        switch (GlobalSingleton::get().musicConfig.playMode) {
        case PlayMode::RandomPlay:  // 随机播放
            if (auto it = GlobalSingleton::get().playQueue.prev()) {
                switchMusic(*it);
                SignalBusSingleton::get().musicResumed();
                break;
            }
        [[fallthrough]];
        case PlayMode::ListLoop:    // 列表循环
        case PlayMode::SinglePlay:  // 单曲播放
        case PlayMode::SingleLoop:  // 单曲循环
            // 应该从 SongListModel 中变
            break;
        case PlayMode::PlayModeCnt: // !保留!
            break;
        }
    }

    /**
     * @brief 设置当前音乐的播放位置 (单位: 毫秒(ms))
     * @param pos 
     */
    static void setMusicPos(qint64 pos) {
        GlobalSingleton::get().music.setPosition(pos);
    }
};

} // namespace HX

#endif // !_HX_MUSIC_COMMAND_H_