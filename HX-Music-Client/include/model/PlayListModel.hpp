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
#include <singleton/ImagePool.h>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>

#include <HXLibs/reflection/EnumName.hpp>

namespace HX {

class PlayListModel : public QAbstractListModel {
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
        uint32_t cnt;      // 歌单歌曲数量
    };
public:
    explicit PlayListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
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
        case CntRole: return playList.cnt;
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
        int sourceRow,
        int count,
        const QModelIndex& destinationParent,
        int destinationRow
    ) override {
        qDebug() << sourceParent << "-->" << destinationParent;
        if (count != 1 || sourceParent.isValid() || destinationParent.isValid())
            return false;
        if (sourceRow < 0 || sourceRow >= static_cast<int>(_playListArr.size()))
            return false;
        if (destinationRow < 0 || destinationRow > static_cast<int>(_playListArr.size()))
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
        auto item = _playListArr[sourceRow];
        _playListArr.erase(_playListArr.begin() + sourceRow);
        _playListArr.insert(
            _playListArr.begin()
                + ((destinationRow > sourceRow) ? destinationRow - 1
                                                : destinationRow),
            item
        );
        endMoveRows();
        return true;
    }

    Q_INVOKABLE void clear() {
        Q_EMIT beginResetModel();
        _playListArr.clear();
        Q_EMIT endResetModel();
    }

private:
    QVector<PlayListData> _playListArr{{
        "我喜欢", 114514, 2233
    }, {
        "你喜欢", 114514, 2233
    }};
};

} // namespace HX

