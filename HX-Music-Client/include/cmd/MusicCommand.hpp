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

#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>
#include <singleton/NetSingleton.hpp>

namespace HX {

/**
 * @brief 音乐相关的命令
 */
struct MusicCommand {
    /**
     * @brief 切换音乐, 并且播放
     * @param it 
     */
    template <bool IsAddQueue = true>
    static void switchMusic(PlayQueue::Type path) {
        // @todo 如果是网络, 则先播放网络的, 同时再下载本地的; 本地下载完成了, 就切换为本地播放.
        if constexpr (IsAddQueue) {
            GlobalSingleton::get().playQueue.push(path);
        }
        bool ok = false;
        uint64_t id = path.toULongLong(&ok);
        if (ok) {
            qDebug() << "播放网络歌曲:" << id;
            GlobalSingleton::get().musicConfig.isPlay = true;
            GlobalSingleton::get().music.switchMusic(
                QUrl{QString{"%1/music/download/%2"}.arg(
                    NetSingleton::get().getBackendUrl(),
                    std::to_string(id)
                )}
            ).play();
        } else {        
            auto fileInfo = QFileInfo{path};
            auto info = MusicInfo{fileInfo};
            GlobalSingleton::get().musicConfig.isPlay = true;
            Q_EMIT SignalBusSingleton::get().newSongLoaded(&info);
            GlobalSingleton::get().music.setLengthInMilliseconds(info.getLengthInMilliseconds());
            GlobalSingleton::get().music.switchMusic(info.filePath()).play();
        }
    }

    /**
     * @brief 修改音量
     * @param volume 
     */
    static void setVolume(float volume) {
        GlobalSingleton::get().musicConfig.volume = volume;
        GlobalSingleton::get().music.setVolume(volume);
        Q_EMIT SignalBusSingleton::get().volumeChanged(volume);
    }

    /**
     * @brief 音乐暂停
     */
    static void pause() {
        GlobalSingleton::get().musicConfig.isPlay = false;
        GlobalSingleton::get().music.pause();
        Q_EMIT SignalBusSingleton::get().musicPaused();
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
        Q_EMIT SignalBusSingleton::get().musicResumed();
    }

    /**
     * @brief 修改播放模式
     */
    static void setPlayMode(PlayMode mode) {
        GlobalSingleton::get().musicConfig.playMode = mode;
        Q_EMIT SignalBusSingleton::get().playModeChanged(mode);
    }

    /**
     * @brief 下一首
     */
    static void nextMusic() {
        Q_EMIT SignalBusSingleton::get().nextMusicByMusicListModel(
            GlobalSingleton::get().musicConfig.listIndex
        );
    }

    /**
     * @brief 上一首
     */
    static void prevMusic() {
        Q_EMIT SignalBusSingleton::get().prevMusicByMusicListModel(
            GlobalSingleton::get().musicConfig.listIndex
        );
    }

    /**
     * @brief 设置当前音乐的播放位置 (单位: 毫秒(ms))
     * @param pos 
     */
    static void setMusicPos(qint64 pos) {
        Q_EMIT GlobalSingleton::get().music.setPosition(pos);
    }
};

} // namespace HX

