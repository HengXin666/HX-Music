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

class MusicListModel : public QAbstractListModel {
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
    explicit MusicListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
        // 歌单更新信号
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::playlistChanged,
            this,
            [this](uint64_t id) {
            auto& playlist = GlobalSingleton::get().guiPlaylist;
            clear();
            _isActiveUpdate = true;
            for (auto const& songData : playlist.songList) {
                addFromNet(songData);
            }
            _isActiveUpdate = false;
            _id = id;
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
     * @brief 拖拽交换接口
     * @param from 
     * @param to 
     * @return bool 是否成功进行交换 
     */
    Q_INVOKABLE bool swapRow(int from, int to) {
        if (from != to
            && from >= 0
            && from < _musicArr.count()
            && to >= 0
            && to < _musicArr.count()
        ) {
            moveItem(static_cast<std::size_t>(from), static_cast<std::size_t>(to));
            beginMoveRows(
                QModelIndex(),
                from,
                from,
                QModelIndex(),
                from > to ? (to) : (to + 1));
            _musicArr.swapItemsAt(from, to);
            endMoveRows();
            return true;
        }
        return false;
    }

    bool moveRows(
        const QModelIndex& sourceParent,
        int from,
        int count,
        const QModelIndex& destinationParent,
        int to
    ) override {
        if (count != 1 || sourceParent.isValid() || destinationParent.isValid())
            return false;
        if (from < 0 || from >= static_cast<int>(_musicArr.size()))
            return false;
        if (to < 0 || to > static_cast<int>(_musicArr.size()))
            return false;
        if (from == to || from + 1 == to)
            return false;
        moveItem(static_cast<std::size_t>(from), static_cast<std::size_t>(to));
        beginMoveRows(
            QModelIndex(),
            from,
            from,
            QModelIndex(),
            (to > from) ? to + 1 : to
        );
        _musicArr.swapItemsAt(from, to);
        endMoveRows();
        return true;
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

    /**
     * @brief 本地歌曲上传到服务器, 并且添加到歌单
     * @param path 
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void addFromPath(QString const& path) {
        std::string localPath = path.toStdString();
        auto nowPlayListId = GlobalSingleton::get().guiPlaylist.id;
        MusicApi::initUploadMusic(
            localPath,
            std::filesystem::path{path.toStdString()}.filename()
        ).thenTry([](container::Try<std::string> t) {
            if (!t) [[unlikely]] {
                MessageController::get().show<MsgType::Error>("初始化上传任务失败:" + t.what());
                t.rethrow();
            }
            auto uuid = t.move();
            log::hxLog.info("uuid:", uuid);
            return std::move(uuid);
        }).thenTry([this, _localPath = std::move(localPath), nowPlayListId](
            container::Try<std::string> t
        ) {
            if (!t) [[unlikely]] {
                t.rethrow();
            }
            log::hxLog.debug("上传文件 (uuid =", t.get(), ")");
            
            MusicApi::uploadMusic(
                _localPath,
                t.move()
            ).thenTry([this, nowPlayListId](container::Try<uint64_t> t) {
                if (!t) [[unlikely]] {
                    MessageController::get().show<MsgType::Error>("上传歌曲获取新歌曲的id失败:" + t.what());
                    t.rethrow();
                }
                log::hxLog.info("获取到歌曲id:", t.get());
                // 添加到当前歌单
                PlaylistApi::addMusic(
                    nowPlayListId, t.get()
                ).thenTry([this, nowPlayListId, musicId = t.get()](auto t) {
                    if (!t) [[unlikely]] {
                        MessageController::get().show<MsgType::Error>("添加歌曲到歌单失败:" + t.what());
                        return;
                    }
                    // 获取歌曲数据, 插入到本歌单. 而不是再次请求.
                    // 因为可能时间差异. 所以需要先判断当前歌单是否还是被添加歌单
                    if (nowPlayListId != GlobalSingleton::get().guiPlaylist.id
                        && nowPlayListId != GlobalSingleton::get().nowPlaylist.id
                    ) {
                        log::hxLog.error("当前没有选择歌单");
                        // 当前没有选择该歌单
                        throw std::runtime_error{"The playlist is currently not selected"};
                    }
                    MusicApi::selectById(
                        musicId
                    ).thenTry([this, nowPlayListId](container::Try<SongInformation> t) {
                        if (!t) [[unlikely]] {
                            t.rethrow();
                        }
                        if (nowPlayListId == GlobalSingleton::get().guiPlaylist.id) {
                            QMetaObject::invokeMethod(
                                QCoreApplication::instance(),
                                [this, songInfo = t.move()] {
                                _isActiveUpdate = true;
                                addFromNet(
                                    GlobalSingleton::get()
                                        .guiPlaylist
                                        .songList
                                        .emplace_back(std::move(songInfo))
                                );
                                _isActiveUpdate = false;
                            });
                        } else if (nowPlayListId == GlobalSingleton::get().nowPlaylist.id) {
                            QMetaObject::invokeMethod(
                                QCoreApplication::instance(),
                                [songInfo = t.move()] {
                                GlobalSingleton::get()
                                    .nowPlaylist
                                    .songList
                                    .emplace_back(std::move(songInfo));
                            });
                        } else [[unlikely]] {
                            // 当前没有选择该歌单
                            throw std::runtime_error{"The playlist is currently not selected"};
                        }
                    }).thenTry([](container::Try<> t) {
                        if (!t) [[unlikely]] {
                            MessageController::get().show<MsgType::Error>("显示新歌曲失败:" + t.what());
                        }
                    });
                });
            });
        });
    }

    void addFromNet(SongInformation const& songInfo) {
        log::hxLog.debug(songInfo);
        addMusic(
            QString::fromStdString(songInfo.musicName),
            [&]{
                QStringList res;
                for (auto const& it : songInfo.singers)
                    res.emplace_back(QString::fromStdString(it));
                return res;
            }(),
            QString::fromStdString(songInfo.musicAlbum),
            QString{"%1"}.arg(songInfo.millisecondsLen / 1000),
            QString::fromStdString(songInfo.path),
            songInfo.id
        );
    }

    Q_INVOKABLE void addMusic(
        QString title,
        QStringList artist,
        QString album,
        QString duration,
        QString const& url,
        uint64_t id = 0
    ) {
        // @todo 待验证
        if (!_isActiveUpdate) {
            // 非主动更新, 即用户更新! 主动更新是对于本类来说的        
            GlobalSingleton::get().guiPlaylist.songList.push_back({
                0,
                url.toStdString(),
                title.toStdString(),
                [&](){
                    std::vector<std::string> res;
                    for (auto const& it : artist) {
                        res.emplace_back(it.toStdString());
                    }
                    return res;
                }(),
                album.toStdString()
            });
            Q_EMIT SignalBusSingleton::get().savePlaylistSignal();
        }
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

    /**
     * @brief 保存歌单接口, 仅本地歌单
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void savePlaylist() {
        MessageController::get().show<MsgType::Error>("保存歌单@todo");
        // decltype(GlobalSingleton::get().guiPlaylist.songList) newSongList;
        // newSongList.reserve(_musicArr.size());
        // for (auto const& it : _musicArr) {
        //     newSongList.push_back({
        //         0,
        //         it.url.toStdString(),
        //         it.title.toStdString(),
        //         [&](){
        //             std::vector<std::string> res;
        //             for (auto const& it : it.artist) {
        //                 res.emplace_back(it.toStdString());
        //             }
        //             return res;
        //         }(),
        //         it.album.toStdString()
        //     });
        // }
        // GlobalSingleton::get().guiPlaylist.songList = std::move(newSongList);
        // Q_EMIT SignalBusSingleton::get().savePlaylistSignal();
    }

    Q_INVOKABLE void clear() {
        Q_EMIT beginResetModel();
        _musicArr.clear();
        Q_EMIT endResetModel();
    }

    Q_INVOKABLE uint64_t getPlaylistId() const noexcept {
        return _id;
    }

private:
    void moveItem(std::size_t from, std::size_t to) {
        PlaylistApi::insertMusic(
            _id,
            from,
            to
        ).thenTry([=, id = _id](auto t) {
            QMetaObject::invokeMethod(
                QCoreApplication::instance(),
                [id, _t = std::move(t), from, to] {
                    if (!_t) [[unlikely]] {
                        MessageController::get().show<MsgType::Error>("插入歌曲失败: " + _t.what());
                    } else {
                        auto& list = GlobalSingleton::get().guiPlaylist.songList;
                        std::swap(list[from], list[to]);
                        if (GlobalSingleton::get().musicConfig.playlistId != id) {
                            return;
                        }
                        if (auto& idx = GlobalSingleton::get().musicConfig.listIndex;
                            idx == from
                        ) {
                            idx = to;
                            Q_EMIT SignalBusSingleton::get().listIndexChanged();
                        }
                    }
                });
        });
    }

    QVector<MusicInfoData> _musicArr{};
    uint64_t _id = 0; // 当前歌单id
    bool _isActiveUpdate = false;
};

} // namespace HX
