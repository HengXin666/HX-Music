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
#include <singleton/OnlineImagePoll.h>
#include <api/MusicApi.hpp>

// debug
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
            [this]() {
                // 必须当前为显示页才可以设置啊
                if (GlobalSingleton::get().musicConfig.playlistId == GlobalSingleton::get().guiPlaylist.id) {
                    Q_EMIT listIndexChanged(getListIndex());
                }
            }
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

        /* newSongLoaded (加载新歌) */
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::newSongLoaded,
            this,
            [this](MusicInformation* info) {
            Q_EMIT playingChanged(GlobalSingleton::get().musicConfig.isPlay);
        });

        // 下首歌
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::nextMusicByMusicListModel,
            this,
            [this](int index) {
            auto const& nowPlaylistSongList = GlobalSingleton::get().nowPlaylist.songList;
            if (nowPlaylistSongList.empty()) {
                return;
            }
            switch (GlobalSingleton::get().musicConfig.playMode) {
            case PlayMode::RandomPlay:  // 随机播放
                if (auto it = GlobalSingleton::get().playQueue.next()) {
                    MusicCommand::switchMusic<false>(*it);
                    Q_EMIT SignalBusSingleton::get().musicResumed();
                    GlobalSingleton::get().musicConfig.listIndex = [&]() -> std::size_t {
                        for (std::size_t i = 0; i < nowPlaylistSongList.size(); ++i)
                            if (*it == nowPlaylistSongList[i].id)
                                return i;
                        [[unlikely]]
                        log::hxLog.error("找不到该id");
                        return {};
                    }();
                    Q_EMIT SignalBusSingleton::get().listIndexChanged();
                    break;
                } else {
                    index = std::uniform_int_distribution<int>{
                        0, static_cast<int>(nowPlaylistSongList.size()) - 1
                    }(_rng);
                }
            [[fallthrough]];
            case PlayMode::ListLoop:    // 列表循环
            case PlayMode::SingleLoop:  // 单曲循环
            {
                auto idx = (index + 1) % nowPlaylistSongList.size();
                MusicCommand::switchMusic(nowPlaylistSongList[idx].id);
                GlobalSingleton::get().musicConfig.listIndex = idx;
                Q_EMIT SignalBusSingleton::get().listIndexChanged();
                break;
            }
            case PlayMode::PlayModeCnt: // !保留!
                break;
            }
        });

        // 上首歌
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::prevMusicByMusicListModel,
            this,
            [this](int index) {
            auto const& nowPlaylistSongList = GlobalSingleton::get().nowPlaylist.songList;
            if (nowPlaylistSongList.empty()) {
                return;
            }
            switch (GlobalSingleton::get().musicConfig.playMode) {
            case PlayMode::RandomPlay:  // 随机播放
                if (auto it = GlobalSingleton::get().playQueue.prev()) {
                    MusicCommand::switchMusic<false>(*it);
                    Q_EMIT SignalBusSingleton::get().musicResumed();
                    GlobalSingleton::get().musicConfig.listIndex = [&]() -> std::size_t {
                        for (std::size_t i = 0; i < nowPlaylistSongList.size(); ++i)
                            if (*it == nowPlaylistSongList[i].id)
                                return i;
                        [[unlikely]]
                        log::hxLog.error("找不到该id");
                        return {};
                    }();
                    Q_EMIT SignalBusSingleton::get().listIndexChanged();
                    break;
                } else {
                    // 也是随机
                    index = std::uniform_int_distribution<int>{
                        0, static_cast<int>(nowPlaylistSongList.size()) - 1
                    }(_rng);
                }
            [[fallthrough]];
            case PlayMode::ListLoop:    // 列表循环
            case PlayMode::SingleLoop:  // 单曲循环
            {
                auto idx = (index - 1 + static_cast<int>(nowPlaylistSongList.size())) 
                              % static_cast<int>(nowPlaylistSongList.size());
                MusicCommand::switchMusic(nowPlaylistSongList[idx].id);
                GlobalSingleton::get().musicConfig.listIndex = idx;
                Q_EMIT SignalBusSingleton::get().listIndexChanged();
                break;
            }
            case PlayMode::PlayModeCnt: // !保留!
                break;
            }
        });

        // === init ===
        QTimer::singleShot(1000, this, [this] {
            // 暂停
            MusicCommand::pause();
            Q_EMIT playingChanged(GlobalSingleton::get().musicConfig.isPlay);
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
     * @param id 音乐id
     */
    Q_INVOKABLE void playMusic(uint64_t id) {
        GlobalSingleton::get().musicConfig.playMusicId = 0;
        MusicCommand::switchMusic(id);
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
     * @brief 跳转到指定时间
     * @param position 单位: 毫秒(ms)
     */
    Q_INVOKABLE void setPosition(qint64 position) {
        MusicCommand::setMusicPos(position);
    }

    /**
     * @brief 设置当前活跃的歌单id (双击播放该歌曲时候)
     * @param id 
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void setPlaylistId(uint64_t id) noexcept {
        // 如果不是当前歌单, 说明是新的活动歌单
        if (id != GlobalSingleton::get().musicConfig.playlistId) {
            auto& maePlaylist = GlobalSingleton::get().nowPlaylist;
            // 活动歌单更新了, 析构之前保存的图片
            for (auto&& it : maePlaylist.songList) {
                OnlineImagePoll::get()->erase(QString{"%1"}.arg(it.id));
            }
            maePlaylist = GlobalSingleton::get().guiPlaylist;
        }
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

    // 扫描音乐库
    Q_INVOKABLE void startScan() {
        MusicApi::startScan([](std::string msg) {
            Q_EMIT SignalBusSingleton::get().backendViewLogUpdated(
                QString::fromStdString(std::move(msg))
            );
        });
    }
Q_SIGNALS:
    void playingChanged(bool isPlaying);
    void volumeChanged(float volume);
    void playModeChanged(PlayMode mode);
    void listIndexChanged(int index);

private:
    std::mt19937 _rng{std::random_device{}()};

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