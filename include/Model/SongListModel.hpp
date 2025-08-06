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
#ifndef _HX_SONG_LIST_MODEL_H_
#define _HX_SONG_LIST_MODEL_H_

#include <QAbstractListModel>

namespace HX {

struct Song {
    QString title;
    QString artist;
    QString coverUrl;
    QString duration;
};

class SongListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum SongRoles {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        CoverUrlRole,
        DurationRole,
    };
    Q_ENUM(SongRoles)

    explicit SongListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return _songs.size();
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= _songs.size())
            return {};

        const auto& song = _songs.at(index.row());
        switch (role) {
        case TitleRole: return song.title;
        case ArtistRole: return song.artist;
        case CoverUrlRole: return song.coverUrl;
        case DurationRole: return song.duration;
        default: return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            { TitleRole, "title" },
            { ArtistRole, "artist" },
            { CoverUrlRole, "coverUrl" },
            { DurationRole, "duration" },
        };
    }

    Q_INVOKABLE void addSong(const QString& title, const QString& artist,
                             const QString& coverUrl, const QString& duration) {
        beginInsertRows({}, _songs.size(), _songs.size());
        _songs.append({ title, artist, coverUrl, duration });
        endInsertRows();
    }

    Q_INVOKABLE void clear() {
        beginResetModel();
        _songs.clear();
        endResetModel();
    }

private:
    QVector<Song> _songs;
};

} // namespace HX

#endif // !_HX_SONG_LIST_MODEL_H_