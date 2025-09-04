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

#include <QObject>

#include <pojo/SongInformation.hpp>
#include <utils/MusicInfo.hpp>

namespace HX {

class MusicInformation : public QObject, private SongInformation {
    Q_OBJECT

    Q_PROPERTY(QString title READ getTitle CONSTANT)
    Q_PROPERTY(QStringList artistList READ getArtistList CONSTANT)
public:
    MusicInformation(QObject* p = nullptr)
        : QObject{p}
        , _img{}
    {}

    MusicInformation(SongInformation&& songInfo, QPixmap img = {})
        : SongInformation{std::move(songInfo)}
        , _img{std::move(img)}
    {}

    MusicInformation(MusicInfo const& musicInfo)
        : MusicInformation{
            SongInformation{
                0,
                musicInfo.filePath().toStdString(),
                musicInfo.getTitle().toStdString(),
                [&]{
                    auto list = musicInfo.getArtistList();
                    std::vector<std::string> singers;
                    for (auto&& it : list) {
                        singers.emplace_back(it.toStdString());
                    }
                    return singers;
                }(),
                musicInfo.getAlbum().toStdString(),
                static_cast<uint64_t>(musicInfo.getLengthInMilliseconds())
            },
            [&] {
                auto opt = musicInfo.getAlbumArtAdvanced();
                return opt ? *opt : QPixmap{};
            }()
        }
    {}

    MusicInformation(const HX::MusicInformation& that)
        : MusicInformation{
            SongInformation{
                that.id,
                that.path,
                that.musicName,
                that.singers,
                that.musicAlbum,
                that.millisecondsLen
            },
            that._img
        }
    {}

    MusicInformation& operator=(const HX::MusicInformation& that) {
        id = that.id;
        path = that.path;
        musicName = that.musicName;
        singers = that.singers;
        musicAlbum = that.musicAlbum;
        millisecondsLen = that.millisecondsLen;
        _img = that._img;
        return *this;
    }

    /**
     * @brief 获取音频标题
     * @return QString `获取失败`则返回文件名
     */
    Q_INVOKABLE QString getTitle() const {
        return QString::fromStdString(musicName);
    }

    /**
     * @brief 获取音频歌手信息列表
     * @return QVector<QString> 每一项就是一个歌手名
     */
    Q_INVOKABLE QStringList getArtistList() const {
        QStringList res;
        for (auto&& it : singers) {
            res.emplace_back(QString::fromStdString(it));
        }
        return res;
    }

    /**
     * @brief 获取音频的总毫秒数
     * @return uint64_t (int)
     */
    Q_INVOKABLE uint64_t getLengthInMilliseconds() const {
        return millisecondsLen;
    }

    Q_INVOKABLE QString filePath() const {
        return QString::fromStdString(path);
    }

    /**
     * @brief 获取歌曲id, 可以通过id请求获取封面
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE uint64_t getId() const {
        return id;
    }
private:
    QPixmap _img; // 封面
};

} // namespace HX

// 注册支持拷贝
Q_DECLARE_METATYPE(HX::MusicInformation);