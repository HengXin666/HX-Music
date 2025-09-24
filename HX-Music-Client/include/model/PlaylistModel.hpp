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
#include <controller/PlaylistController.h>
#include <api/PlaylistApi.hpp>

#include <HXLibs/reflection/EnumName.hpp>

namespace HX {

class PlaylistModel : public QAbstractListModel {
    Q_OBJECT

    enum PlayListRoles {
        NameRole = Qt::UserRole + 1,
        IdRole,
        CntRole,
        PlayListTypeRole,
    };

    struct PlayListData {
        QString name;      // 歌单名称
        uint64_t id;       // 歌单id
        uint64_t cnt;      // 歌单歌曲数量
    };
public:
    explicit PlaylistModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::updatePlaylistList,
            this,
            [this](uint64_t newId) {
                if (!newId) {
                    // 全量更新
                    updateAllPlaylistInfoList();
                } else {
                    // 仅更新 id = newId
                    PlaylistApi::getPlaylistInfo(newId)
                        .thenTry([this](auto t) {
                            if (!t) [[unlikely]] {
                                MessageController::get().show<MsgType::Error>(
                                    "更新歌单简介失败:" + t.what());
                                return;
                            }
                            addData(t.move());
                        });
                }
            }
        );
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return _playListArr.size();
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= _playListArr.size())
            return {};

        const auto& playList = _playListArr.at(index.row());
        switch (role) {
        case NameRole: return playList.name;
        case IdRole: return QString{"%1"}.arg(playList.id);
        case CntRole: return QString{"%1"}.arg(playList.cnt);
        default: return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            { NameRole, "name" },
            { IdRole, "id" },
            { CntRole, "cnt" },
        };
    }

    void addData(PlaylistInfo const& info) {
        Q_EMIT beginInsertRows({}, _playListArr.size(), _playListArr.size());
        _playListArr.emplace_back(
            QString::fromStdString(info.name),
            info.id,
            info.cnt
        );
        PlaylistController::get()._playListArr.push_back({
            info.id,
            QString::fromStdString(info.name)
        });
        Q_EMIT endInsertRows();
    }

    // 更新歌单列表
    Q_INVOKABLE void updateAllPlaylistInfoList() {
        (_isShowCreated 
            ? PlaylistApi::selectUserCreatedAllPlaylist()
            : PlaylistApi::selectUserSavedAllPlaylist()
        ).thenTry([this](auto t) {
            if (!t) [[unlikely]] {
                MessageController::get().show<MsgType::Error>(
                    "更新歌单简介列表失败:" + t.what());
                return;
            }
            QMetaObject::invokeMethod(
                QCoreApplication::instance(),
                [this, infoList = t.move()] {
                clear();
                PlaylistController::get()._playListArr.clear();
                for (auto const& info : infoList) {
                    addData(info);
                }
            });
        });
    }

    /**
     * @brief 获取指定索引的元素的id
     * @param idx 
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE uint64_t getId(std::size_t idx) const {
        return _playListArr[idx].id;
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
            && from < _playListArr.count()
            && to >= 0
            && to < _playListArr.count()
        ) {
            beginMoveRows(
                QModelIndex(),
                from,
                from,
                QModelIndex(),
                from > to ? (to) : (to + 1));
            _playListArr.swapItemsAt(from, to);
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
        if (from < 0 || from >= static_cast<int>(_playListArr.size()))
            return false;
        if (to < 0 || to > static_cast<int>(_playListArr.size()))
            return false;
        if (from == to || from + 1 == to)
            return false;

        beginMoveRows(
            QModelIndex(),
            from,
            from,
            QModelIndex(),
            (to > from) ? to + 1 : to
        );
        _playListArr.swapItemsAt(from, to);
        endMoveRows();
        return true;
    }

    Q_INVOKABLE void clear() {
        Q_EMIT beginResetModel();
        _playListArr.clear();
        Q_EMIT endResetModel();
    }

    Q_INVOKABLE void setShowCreatedPlaylist(bool isShowCreated) {
        _isShowCreated = isShowCreated;
    }

    // 保存歌单顺序
    Q_INVOKABLE void savePlaylist() const {
        std::vector<uint64_t> idList;
        for (auto const& v : _playListArr)
            idList.emplace_back(v.id);
        (_isShowCreated
            ? PlaylistApi::updateCreatedPlaylistOrder(std::move(idList))
            : PlaylistApi::updateSavedPlaylistOrder(std::move(idList))
        ).thenTry([](container::Try<> t) {
            if (!t) [[unlikely]] {
                MessageController::get().show<MsgType::Error>(
                    "保存歌单顺序失败: " + t.what()
                );
            }
        });
    }

private:
    QVector<PlayListData> _playListArr{};
    bool _isShowCreated = true; // 当前是否展示是用户创建的歌单
};

} // namespace HX

