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
#include <utils/MusicInfo.hpp>
#include <singleton/ImagePool.h>

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
public:
    explicit MusicListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {}

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
        emit beginInsertRows({}, _musicArr.size(), _musicArr.size());
        _musicArr.append({
            std::move(title),
            std::move(artist),
            std::move(album),
            std::move(duration),
            url
        });
        emit endInsertRows();
    }

    Q_INVOKABLE void clear() {
        emit beginResetModel();
        _musicArr.clear();
        emit endResetModel();
    }

private:
    QVector<MusicInfoData> _musicArr;
};

} // namespace HX

#endif // !_HX_MUSIC_LIST_MODEL_H_