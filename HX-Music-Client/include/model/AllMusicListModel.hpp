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

#include <QAbstractListModel>
#include <cmd/MusicCommand.hpp>
#include <utils/MusicInfo.hpp>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>
#include <api/PlaylistApi.hpp>

namespace HX {

class AllMusicListModel : public QAbstractListModel {
    Q_OBJECT

    enum MusicRoles {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        AlbumRole,
        DurationRole,
        UrlRole,
        IdRole
    };

    struct MusicInfoData {
        QString title;          // 歌名
        QStringList artist;     // 歌手列表
        QString album;          // 专辑
        QString duration;       // 时长 (单位: 秒(s))
        uint64_t id = 0;        // 歌曲id, 仅网络有效
    };
public:
    explicit AllMusicListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
        loadMoreRequested();
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
        case IdRole: return QString{"%1"}.arg(music.id);
        default: return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            { TitleRole, "title" },
            { ArtistRole, "artist" },
            { AlbumRole, "album" },
            { DurationRole, "duration" },
            { IdRole, "id" },
        };
    }

    /**
     * @brief 歌曲Id
     * @param row  
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE uint64_t getUrl(int row) const {
        if (row < 0 || row >= _musicArr.size()) {
            log::hxLog.error("getUrl 越界:", row);
            return {};
        }
        return _musicArr[row].id;
    }

    Q_INVOKABLE void addMusic(
        QString title,
        QStringList artist,
        QString album,
        QString duration,
        QString const& url,
        uint64_t id = 0
    ) {
        Q_EMIT beginInsertRows({}, _musicArr.size(), _musicArr.size());
        _musicArr.append({
            std::move(title),
            std::move(artist),
            std::move(album),
            std::move(duration),
            id
        });
        Q_EMIT endInsertRows();
    }

    Q_INVOKABLE void clear() {
        Q_EMIT beginResetModel();
        _musicArr.clear();
        Q_EMIT endResetModel();
    }

    Q_INVOKABLE void loadMoreRequested() {
        MusicApi::selectMusic(
            _beginId, 10
        ).thenTry([this](container::Try<std::vector<SongInformation>> t) {
            if (!t) {
                MessageController::get().show<MsgType::Error>("加载歌曲失败: " + t.what());
                return;
            }
            QMetaObject::invokeMethod(
                QCoreApplication::instance(),
                [this, songList = t.move()] {
                    for (auto&& v : songList) {
                        addMusic(
                            QString::fromStdString(v.musicName),
                            [&]{
                                QStringList res;
                                for (auto&& str : v.singers)
                                    res.append(QString::fromStdString(str));
                                return res;
                            }(),
                            QString::fromStdString(v.musicAlbum),
                            QString{"%1"}.arg(v.millisecondsLen),
                            QString{"%1"}.arg(v.id),
                            v.id
                        );
                    }
                    if (songList.size()) {
                        _beginId = songList.back().id;
                    }
                }
            );
        });
    }
private:
    QVector<MusicInfoData> _musicArr{};
    uint64_t _beginId = 0;
};

} // namespace HX
