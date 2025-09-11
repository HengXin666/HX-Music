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
        IdRole
    };

    struct MusicInfoData {
        QString title;          // 歌名
        QStringList artist;     // 歌手列表
        QString album;          // 专辑
        QString duration;       // 时长 (单位: 秒(s))
        QString url;            // 歌曲路径, 如果是网络歌单则是服务器内部的相对路径
                                // 如果是本地歌单, 则是本机的绝对路径
        uint64_t id = 0;        // 歌曲id, 仅网络有效
    };

    // 通过 URL 查找 索引
    int findByUrl(QString const& url) const noexcept {
        for (int i = 0; auto const& it : _musicArr) {
            if (getUrl(i) == url) {
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
                MusicCommand::switchMusic(getUrl(idx));
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
                MusicCommand::switchMusic(getUrl(idx));
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
        case UrlRole: return music.url;
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
            { UrlRole, "url" },
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
        int sourceRow,
        int count,
        const QModelIndex& destinationParent,
        int destinationRow
    ) override {
        // qDebug() << sourceParent << "-->" << destinationParent;
        if (count != 1 || sourceParent.isValid() || destinationParent.isValid())
            return false;
        if (sourceRow < 0 || sourceRow >= static_cast<int>(_musicArr.size()))
            return false;
        if (destinationRow < 0 || destinationRow > static_cast<int>(_musicArr.size()))
            return false;
        if (sourceRow == destinationRow || sourceRow + 1 == destinationRow)
            return false;

        beginMoveRows(
            QModelIndex(),
            sourceRow,
            sourceRow,
            QModelIndex(),
            (destinationRow > sourceRow) ? destinationRow + 1 : destinationRow
        );
        auto item = _musicArr[sourceRow];
        _musicArr.erase(_musicArr.begin() + sourceRow);
        _musicArr.insert(
            _musicArr.begin()
                + ((destinationRow > sourceRow) ? destinationRow - 1
                                                : destinationRow),
            item
        );
        endMoveRows();
        return true;
    }

    /**
     * @brief 获取 URL, 如果为网络歌单, 则返回歌曲Id, 否则为本地路径
     * @param row 
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE QString getUrl(int row) const {
        if (row < 0 || row >= _musicArr.size()) {
            log::hxLog.error("getUrl 越界:", row);
            return {};
        }
        return QString{"%1"}.arg(_musicArr[row].id);
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
                log::hxLog.error(t.what());
                t.rethrow();
            }
            auto uuid = t.move();
            log::hxLog.info("uuid:", uuid);
            return std::move(uuid);
        }).thenTry([this, _localPath = std::move(localPath), nowPlayListId](
            container::Try<std::string> t
        ) {
            if (!t) [[unlikely]] {
                log::hxLog.error(t.what());
                t.rethrow();
            }
            log::hxLog.debug("上传文件 (uuid =", t.get(), ")");
            
            MusicApi::uploadMusic(
                _localPath,
                t.move()
            ).thenTry([this, nowPlayListId](container::Try<uint64_t> t) {
                if (!t) [[unlikely]] {
                    log::hxLog.error("上传歌曲获取新歌曲的id失败:", t.what());
                    t.rethrow();
                }
                log::hxLog.info("获取到歌曲id:", t.get());
                // 添加到当前歌单
                PlaylistApi::addMusic(
                    nowPlayListId, t.get()
                ).thenTry([this, nowPlayListId, musicId = t.get()](auto t) {
                    if (!t) [[unlikely]] {
                        log::hxLog.error("添加歌曲到歌单失败:", t.what());
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
                        // @todo 此处应该要同步一下, 通过 QT 的同步?
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
                            log::hxLog.error("显示新歌曲失败:", t.what());
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
            url,
            id
        });
        Q_EMIT endInsertRows();
    }

    /**
     * @brief 保存歌单接口, 仅本地歌单
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void savePlaylist() {
        decltype(GlobalSingleton::get().guiPlaylist.songList) newSongList;
        newSongList.reserve(_musicArr.size());
        for (auto const& it : _musicArr) {
            newSongList.push_back({
                0,
                it.url.toStdString(),
                it.title.toStdString(),
                [&](){
                    std::vector<std::string> res;
                    for (auto const& it : it.artist) {
                        res.emplace_back(it.toStdString());
                    }
                    return res;
                }(),
                it.album.toStdString()
            });
        }
        GlobalSingleton::get().guiPlaylist.songList = std::move(newSongList);
        Q_EMIT SignalBusSingleton::get().savePlaylistSignal();
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
    std::mt19937 _rng{std::random_device{}()};
    QVector<MusicInfoData> _musicArr{};
    uint64_t _id = 0; // 当前歌单id
    bool _isActiveUpdate = false;
};

} // namespace HX

