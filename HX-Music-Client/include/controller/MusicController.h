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
#include <QTimer>

#include <cmd/MusicCommand.hpp>

// debug
#include <QDebug>
#include <HXLibs/log/Log.hpp>

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
    MusicController(QObject* p = nullptr)
        : QObject{p} 
    {
        // 订阅更新索引
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::listIndexChanged,
            this,
            [this]() { Q_EMIT listIndexChanged(getListIndex()); }
        );

        // 歌单更新信号
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::playlistChanged,
            this,
            [this](uint64_t id) {
            // 延迟调用, 因为 MusicListModel 可能还没有更新好, 我们延迟到下一个事件循环
            QTimer::singleShot(0, this, [this, id]() {
                if (id != GlobalSingleton::get().musicConfig.playlistId) {
                    Q_EMIT listIndexChanged(-1);
                } else {
                    Q_EMIT listIndexChanged(getListIndex());
                }
            });
        });
    }

    Q_INVOKABLE void prev() {
        qDebug() << "上一首";
        MusicCommand::prevMusic();
        Q_EMIT playingChanged(GlobalSingleton::get().musicConfig.isPlay = true);
    }

    Q_INVOKABLE void next() {
        qDebug() << "下一首";
        MusicCommand::nextMusic();
        Q_EMIT playingChanged(GlobalSingleton::get().musicConfig.isPlay = true);
    }

    /**
     * @brief 播放/暂停
     */
    Q_INVOKABLE void togglePause() {
        GlobalSingleton::get().musicConfig.isPlay
            ? qDebug() << "暂停了..."
            : qDebug() << "播放!";
        GlobalSingleton::get().musicConfig.isPlay
            ? MusicCommand::pause()
            : MusicCommand::resume();
        Q_EMIT playingChanged(GlobalSingleton::get().musicConfig.isPlay);
    }

    /**
     * @brief 播放音乐
     * @param path 音乐路径
     */
    Q_INVOKABLE void playMusic(QString const& path) {
        MusicCommand::switchMusic(path);
        Q_EMIT playingChanged(true);
    }

    /**
     * @brief 修改播放模式
     */
    Q_INVOKABLE PlayMode getPlayMode() const {
        return GlobalSingleton::get().musicConfig.playMode;
    }

    /**
     * @brief 修改播放模式
     */
    Q_INVOKABLE void setPlayMode(PlayMode mode) {
        MusicCommand::setPlayMode(mode);
        Q_EMIT playModeChanged(mode);
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
     * @brief 获取音量大小
     * @return 0.00f ~ 1.00f (百分比) 
     */
    Q_INVOKABLE float getVolume() const {
        return GlobalSingleton::get().musicConfig.volume;
    }

    /**
     * @brief 设置音量大小
     * @param volume 0.00f ~ 1.00f (百分比)
     */
    Q_INVOKABLE void setVolume(float volume) {
        MusicCommand::setVolume(volume);
        Q_EMIT volumeChanged(volume);
    }

    /**
     * @brief 获取歌曲播放到的位置 (从配置文件恢复)
     * @return Q_INVOKABLE qint64
     */
    Q_INVOKABLE qint64 getTheLastPlayedPosition() const {
        return GlobalSingleton::get().musicConfig.position;
    }

    /**
     * @brief 跳转到指定时间
     * @param position 单位: 毫秒(ms)
     */
    Q_INVOKABLE void setPosition(qint64 position) {
        MusicCommand::setMusicPos(position);
    }

    /**
     * @brief 设置当前活跃的歌单id
     * @param id 
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void setPlaylistId(uint64_t id) noexcept {
        GlobalSingleton::get().musicConfig.playlistId = id;
    }

    /**
     * @brief 获取播放状态
     * @return true 正在播放
     * @return false 已暂停
     */
    bool getPlaying() const noexcept {
        return GlobalSingleton::get().musicConfig.isPlay;
    }

    /**
     * @brief 设置播放状态
     * @param val 
     */
    void setPlaying(bool val) noexcept {
        GlobalSingleton::get().musicConfig.isPlay = val;
        Q_EMIT playingChanged(val);
    }

    /**
     * @brief 获取当前所在歌单的选择音乐的索引 (-1为无效, 即未选择)
     * @return int 
     */
    int getListIndex() const noexcept {
        return GlobalSingleton::get().musicConfig.listIndex;
    }

    /**
     * @brief 设置当前所在歌单的选择音乐的索引 (-1为无效, 即未选择)
     * @param index 
     */
    void setListIndex(int index) noexcept {
        GlobalSingleton::get().musicConfig.listIndex = index;
    }
Q_SIGNALS:
    void playingChanged(bool isPlaying);
    void volumeChanged(float volume);
    void playModeChanged(PlayMode mode);
    void listIndexChanged(int index);

private:
    Q_PROPERTY(
        bool isPlaying 
        READ getPlaying 
        WRITE setPlaying 
        NOTIFY playingChanged
    );
    Q_PROPERTY(
        float volume 
        READ getVolume 
        WRITE setVolume 
        NOTIFY volumeChanged
    );
    Q_PROPERTY(
        PlayMode playMode 
        READ getPlayMode
        WRITE setPlayMode 
        NOTIFY playModeChanged
    );
    Q_PROPERTY(
        int listIndex 
        READ getListIndex
        WRITE setListIndex 
        NOTIFY listIndexChanged
    );
};

} // namespace HX

#endif // !_HX_MUSIC_CONTROLLER_H_