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
#ifndef _HX_MUSIC_CONTROLLER_H_
#define _HX_MUSIC_CONTROLLER_H_

#include <QObject>

#include <cmd/MusicCommand.hpp>

#include <QDebug>

namespace HX {

/**
 * @brief 音乐控制器
 *        用于控制音乐播放状态的QML交互对象
 * 可以控制:
 *  - 播放/暂停
 *  - 上一首/下一首
 *  - 设置播放模式: 单曲循环/顺序/随机
 *  - 拖动条跳转到某位置
 */
class MusicController : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void prev() {
        qDebug() << "上一首";
        MusicCommand::prevMusic();
    }
    Q_INVOKABLE void next() {
        qDebug() << "下一首";
        MusicCommand::nextMusic();
    }

    /**
     * @brief 播放/暂停
     */
    Q_INVOKABLE void togglePause() { 
        GlobalSingleton::get().musicConfig.isPlay
            ? MusicCommand::pause()
            : MusicCommand::resume();
    }

    /**
     * @brief 修改播放模式
     */
    Q_INVOKABLE void setPlayMode(PlayMode mode) {
        MusicCommand::setPlayMode(mode);
    }

    /**
     * @brief 获取当前播放的位置(单位: 毫秒(ms))
     * @return qint64 
     */
    Q_INVOKABLE qint64 getNowPos() const {
        return GlobalSingleton::get().music.getNowPos();
    }

    /**
     * @brief 获取当前音频的总毫秒数
     * @return qint64 
     */
    Q_INVOKABLE qint64 getLengthInMilliseconds() const {
        return GlobalSingleton::get().music.getLengthInMilliseconds();
    }

    /**
     * @brief 设置音量大小
     * @param volume 0.00f ~ 1.00f (百分比)
     */
    Q_INVOKABLE void setVolume(float volume) {
        MusicCommand::setVolume(volume);
    }

    /**
     * @brief 跳转到指定时间
     * @param position 单位: 毫秒(ms)
     */
    Q_INVOKABLE void setPosition(qint64 position) {
        MusicCommand::setMusicPos(position);
    }
};

} // namespace HX

#endif // !_HX_MUSIC_CONTROLLER_H_