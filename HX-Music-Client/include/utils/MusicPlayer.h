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
#ifndef _HX_MUSIC_PLAYER_H_
#define _HX_MUSIC_PLAYER_H_

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QUrl>

#include <utils/MusicInfo.hpp>

namespace HX {

/**
 * @brief 音乐播放类
 */
class MusicPlayer : public QObject {
    Q_OBJECT
public:
    explicit MusicPlayer();

    /**
     * @brief 切换音乐
     * @param musicPath 目标音乐的路径
     * @return MusicPlayer& 
     */
    MusicPlayer& switchMusic(QString const& musicPath) {
        _player.setSource(QUrl::fromLocalFile(musicPath));
        return *this;
    }

    /**
     * @brief 切换音乐
     * @param url 目标音乐的网络链接
     * @return MusicPlayer& 
     */
    MusicPlayer& switchMusic(QUrl const& url) {
        _player.setSource(url);
        return *this;
    }

    /**
     * @brief 播放音乐
     */
    void play() {
        _player.play();
    }

    /**
     * @brief 暂停音乐
     */
    void pause() {
        _player.pause();
    }

    /**
     * @brief 跳转到指定时间
     * @param position 单位: 毫秒(ms)
     */
    MusicPlayer& setPosition(qint64 position) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        // 50ms 内不重复 Seek, 避免频繁 Range 请求
        if (now - _lastSeekTime > 50 && position != _lastPosition) {
            _player.setPosition(position);
            _lastPosition = position;
            _lastSeekTime = now;
        }
        return *this;
    }

    /**
     * @brief 设置音量大小
     * @param volume 0.00f ~ 1.00f
     * @return MusicPlayer& 
     */
    MusicPlayer& setVolume(float volume) {
        _player.audioOutput()->setVolume(volume);
        return *this;
    }

    /**
     * @brief 获取当前播放的位置(单位: 毫秒(ms))
     */
    qint64 getNowPos() const {
        return _player.position();
    }

    /**
     * @brief 获取当前音频的总毫秒数
     * @return qint64 
     */
    qint64 getLengthInMilliseconds() const {
        return _lengthInMilliseconds;
    }

    /**
     * @brief 设置当前音频的总毫秒数
     * @param pos 
     */
    void setLengthInMilliseconds(int pos) {
        _lengthInMilliseconds = pos;
    }

private:
    QMediaPlayer _player;
    int _lengthInMilliseconds{0}; // 毫秒长度
    qint64 _lastPosition = -1;
    qint64 _lastSeekTime = 0;
};

} // namespace HX

#endif // !_HX_MUSIC_PLAYER_H_