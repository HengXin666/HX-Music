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

namespace HX {

class MusicListModel : public QAbstractListModel {
    Q_OBJECT

    enum MusicRoles {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        AlbumRole,
        DurationRole,
    };

    struct MusicInfoData {
        QString title;
        QString artist;
        QString album;
        QString duration;
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
        default: return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            { TitleRole, "title" },
            { ArtistRole, "artist" },
            { AlbumRole, "album" },
            { DurationRole, "duration" },
        };
    }

    Q_INVOKABLE void addFromPath(QString const& path) {
        MusicInfo musicInfo{QFileInfo{path}};
        qDebug() << "添加:" << musicInfo.getTitle();
        addMusic(
            musicInfo.getTitle(),
            musicInfo.getArtist(),
            musicInfo.getAlbum(),
            QString{"%1"}.arg(musicInfo.getLengthInSeconds())
        );
        qDebug() << "添加完成:" << musicInfo.getArtist();
        for (auto& _ : _musicArr)
            qDebug() << _.artist;
    }

    Q_INVOKABLE void addMusic(const QString& title, const QString& artist,
                              const QString& album, const QString& duration) {
        emit beginInsertRows({}, _musicArr.size(), _musicArr.size());
        _musicArr.append({ title, artist, album, duration });
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