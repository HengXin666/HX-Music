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

#include <memory>

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QUrl>

class MusicPlayer {
public:
    explicit MusicPlayer()
        : _player(std::make_unique<QMediaPlayer>())
    {
        auto audioOutput = new QAudioOutput;
        _player->setAudioOutput(audioOutput);
        audioOutput->setVolume(50);
    }

    MusicPlayer& switchMusic(QString const& musicPath) {
        _player->setSource(QUrl::fromLocalFile(musicPath));
        return *this;
    }

    void play() const {
        _player->play();
    }

    /**
     * @brief 跳转到指定时间
     * @param position 单位: 毫秒(ms)
     */
    MusicPlayer& setPosition(qint64 position) {
        _player->setPosition(position);
        return *this;
    }
private:
    std::unique_ptr<QMediaPlayer> _player;
};

#endif // !_HX_MUSIC_PLAYER_H_