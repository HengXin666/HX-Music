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
#ifndef _HX_MUSIC_LIST_MODEL_H_
#define _HX_MUSIC_LIST_MODEL_H_

#include <QAbstractListModel>
#include <cmd/MusicCommand.hpp>
#include <utils/MusicInfo.hpp>
#include <singleton/ImagePool.h>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>

#include <random>

namespace HX {

class MusicListModel : public QAbstractListModel {
    Q_OBJECT

    enum MusicRoles {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        AlbumRole,
        DurationRole,
        UrlRole,
    };

    struct MusicInfoData {
        QString title;          // 歌名
        QStringList artist;     // 歌手列表
        QString album;          // 专辑
        QString duration;       // 时长 (单位: 秒(s))
        QString url;            // path && img.url && imgPool-id && 配置文件歌曲路径
    };

    int findByUrl(QString const& url) const noexcept {
        for (int i = 0; auto const& it : _musicArr) {
            if (it.url == url) {
                return i;
            }
            ++i;
        }
        return -1;
    }
public:
    explicit MusicListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
        // 下首歌
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::nextMusicByMusicListModel,
            this,
            [this](int index) {
            if (_musicArr.empty()) {
                return;
            }
            switch (GlobalSingleton::get().musicConfig.playMode) {
            case PlayMode::RandomPlay:  // 随机播放
                if (auto it = GlobalSingleton::get().playQueue.next()) {
                    MusicCommand::switchMusic<false>(*it);
                    Q_EMIT SignalBusSingleton::get().musicResumed();
                    GlobalSingleton::get().musicConfig.listIndex = findByUrl(*it);
                    Q_EMIT SignalBusSingleton::get().listIndexChanged();
                    break;
                } else {
                    index = std::uniform_int_distribution<int>{
                        0, static_cast<int>(_musicArr.size()) - 1
                    }(_rng);
                }
            [[fallthrough]];
            case PlayMode::ListLoop:    // 列表循环
            case PlayMode::SingleLoop:  // 单曲循环
            {
                auto idx = (index + 1) % static_cast<int>(_musicArr.size());
                MusicCommand::switchMusic(_musicArr[idx].url);
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
            if (_musicArr.empty()) {
                return;
            }
            switch (GlobalSingleton::get().musicConfig.playMode) {
            case PlayMode::RandomPlay:  // 随机播放
                if (auto it = GlobalSingleton::get().playQueue.prev()) {
                    MusicCommand::switchMusic<false>(*it);
                    Q_EMIT SignalBusSingleton::get().musicResumed();
                    GlobalSingleton::get().musicConfig.listIndex = findByUrl(*it);
                    Q_EMIT SignalBusSingleton::get().listIndexChanged();
                    break;
                } else {
                    // 也是随机
                    index = std::uniform_int_distribution<int>{
                        0, static_cast<int>(_musicArr.size()) - 1
                    }(_rng);
                }
            [[fallthrough]];
            case PlayMode::ListLoop:    // 列表循环
            case PlayMode::SingleLoop:  // 单曲循环
            {
                auto idx = (index - 1 + static_cast<int>(_musicArr.size())) 
                              % static_cast<int>(_musicArr.size());
                MusicCommand::switchMusic(_musicArr[idx].url);
                GlobalSingleton::get().musicConfig.listIndex = idx;
                Q_EMIT SignalBusSingleton::get().listIndexChanged();
                break;
            }
            case PlayMode::PlayModeCnt: // !保留!
                break;
            }
        });

        // 歌单更新信号
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::playlistChanged,
            this,
            [this]() {
            auto& playlist = GlobalSingleton::get().musicList;
            clear();
            _isActiveUpdate = true;
            for (auto const& songData : playlist.songList) {
                auto const& url = songData.url;
                addFromPath(QString::fromStdString(url));
            }
            _isActiveUpdate = false;
        });
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return _musicArr.size();
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= _musicArr.size())
            return {};

        const auto& music = _musicArr.at(index.row());
        switch (role) {
        case TitleRole: return music.title;
        case ArtistRole: return music.artist;
        case AlbumRole: return music.album;
        case DurationRole: return music.duration;
        case UrlRole: return music.url;
        default: return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            { TitleRole, "title" },
            { ArtistRole, "artist" },
            { AlbumRole, "album" },
            { DurationRole, "duration" },
            { UrlRole, "url" },
        };
    }

    Q_INVOKABLE QString getUrl(int row) const {
        return _musicArr[row].url;
    }

    Q_INVOKABLE void addFromPath(QString const& path) {
        MusicInfo musicInfo{QFileInfo{path}};
        addMusic(
            musicInfo.getTitle(),
            musicInfo.getArtistList(),
            musicInfo.getAlbum(),
            QString{"%1"}.arg(musicInfo.getLengthInSeconds()),
            path
        );
        if (auto img = musicInfo.getAlbumArtAdvanced()) {
            ImagePoll::get()->add(path, img->toImage());
        }
    }

    Q_INVOKABLE void addMusic(
        QString title,
        QStringList artist,
        QString album,
        QString duration,
        QString const& url
    ) {
        Q_EMIT beginInsertRows({}, _musicArr.size(), _musicArr.size());
        _musicArr.append({
            std::move(title),
            std::move(artist),
            std::move(album),
            std::move(duration),
            url
        });
        Q_EMIT endInsertRows();
        if (!_isActiveUpdate) {
            // 非主动更新, 即用户更新! 主动更新是对于本类来说的        
            GlobalSingleton::get().musicList.songList.push_back({
                url.toStdString()
            });
            Q_EMIT SignalBusSingleton::get().savePlaylistSignal();
        }
    }

    Q_INVOKABLE void clear() {
        Q_EMIT beginResetModel();
        _musicArr.clear();
        Q_EMIT endResetModel();
    }

private:
    std::mt19937 _rng{std::random_device{}()};
    QVector<MusicInfoData> _musicArr{};
    bool _isActiveUpdate = false;
};

} // namespace HX

#endif // !_HX_MUSIC_LIST_MODEL_H_