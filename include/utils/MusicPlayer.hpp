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

#include <QDebug>

/**
 * @brief 音乐播放类
 */
class MusicPlayer : public QObject {
    Q_OBJECT
public:
    explicit MusicPlayer()
        : _player()
    {
        auto audioOutput = new QAudioOutput(&_player);
        _player.setAudioOutput(audioOutput);
        audioOutput->setVolume(100);
        connect(&_player, &QMediaPlayer::mediaStatusChanged, this, [this](
            QMediaPlayer::MediaStatus mediaState
        ){
            if (mediaState != QMediaPlayer::MediaStatus::EndOfMedia) {
                return;
            }
            qDebug() << "播放完毕";
        });
    }

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
     * @brief 播放音乐
     */
    void play() {
        _player.play();
    }

    /**
     * @brief 跳转到指定时间
     * @param position 单位: 毫秒(ms)
     */
    MusicPlayer& setPosition(qint64 position) {
        _player.setPosition(position);
        return *this;
    }

    /**
     * @brief 设置音量大小
     * @param volume 0 ~ 100
     * @return MusicPlayer& 
     */
    MusicPlayer& setVolume(float volume) {
        _player.audioOutput()->setVolume(volume);
        return *this;
    }
private:
    QMediaPlayer _player;
};

#endif // !_HX_MUSIC_PLAYER_H_