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
#ifndef _HX_SIGNAL_BUS_SINGLETON_H_
#define _HX_SIGNAL_BUS_SINGLETON_H_

#include <QObject>
#include <QMutex>

#include <config/MusicConfig.hpp>
#include <utils/MusicInfo.hpp>

namespace HX {

/**
 * @brief 信号总线单例
 */
class SignalBusSingleton : public QObject {
    Q_OBJECT

    explicit SignalBusSingleton() noexcept {}
    SignalBusSingleton& operator=(SignalBusSingleton&&) = delete;

public:
    /**
     * @brief `线程安全`获取信号总线单例
     * @return SignalBusSingleton& 
     */
    inline static SignalBusSingleton& get() {
        static QMutex mutex{};
        QMutexLocker _{&mutex};
        static SignalBusSingleton singleton{};
        return singleton;
    }

Q_SIGNALS:
    /**
     * @brief 加载新歌信号
     * @param info 歌曲信息
     */
    void newSongLoaded(HX::MusicInfo const& info);

    /**
     * @brief 音量变化
     * @param volume 新的音量
     */
    void volumeChanged(float volume);

    /**
     * @brief 音乐暂停
     */
    void musicPaused();

    /**
     * @brief 音乐播放、音乐继续
     */
    void musicResumed();

    /**
     * @brief 播放模式变化
     */
    void playModeChanged(PlayMode mode);

    /**
     * @brief 播放位置变化
     * @param pos 当前播放位置 (单位: 毫秒(ms))
     */
    void musicPlayPosChanged(qint64 pos);

    /**
     * @brief 歌词界面是否上锁信号
     * @param isLock true: 上锁
     */
    void lyricViewLockChanged(bool isLock);
};

} // namespace HX

#endif // !_HX_SIGNAL_BUS_SINGLETON_H_