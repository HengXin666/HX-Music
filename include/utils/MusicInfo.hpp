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
#ifndef _HX_MUSIC_INFO_H_
#define _HX_MUSIC_INFO_H_

#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/tpropertymap.h> // PropertyMap

#include <QTime>
#include <QFileInfo>
#include <QPixmap>
#include <QString>
#include <QRegularExpression>

#include <utils/FileInfo.hpp>

namespace HX {


class MusicInfo : public QObject {
    Q_OBJECT
public:
    explicit MusicInfo(QFileInfo const& fileInfo) 
        : _fileInfo(fileInfo)
        , mpegFile(QFile::encodeName(_fileInfo.canonicalFilePath()).constData())
    {}

    /**
     * @brief 获取音频标题
     * @return QString `获取失败`则返回文件名
     */
    QString getTitle() const {
        if (mpegFile.isNull() || !mpegFile.tag()) {
            return _fileInfo.fileName();
        }
        auto res = QString::fromStdString(mpegFile.tag()->title().to8Bit(true));
        return res.isEmpty() ? _fileInfo.fileName() : res;
    }

    /**
     * @brief 获取音频歌手信息
     * @param errRes 获取失败返回的值
     * @return QString
     */
    QString getArtist(QString&& errRes = "") const {
        if (mpegFile.isNull() || !mpegFile.tag()) {
            return errRes;
        }
        return QString::fromStdString(mpegFile.tag()->artist().to8Bit(true));
    }

    /**
     * @brief 获取音频歌手信息列表
     * @param errRes 
     * @return QVector<QString> 每一项就是一个歌手名
     */
    QStringList getArtistList(QStringList&& errRes = {}) const {
        if (mpegFile.isNull()) {
            return errRes;
        }
        auto* tag = mpegFile.tag();
        if (!tag) {
            return errRes;
        }
        
        const TagLib::PropertyMap& map = tag->properties();
        if (!map.contains("ARTIST")) {
            return errRes;
        }
        const auto& values = map["ARTIST"];
        if (values.size() > 1) {
            QStringList res;
            for (const auto& val : values) {
                qDebug() << QString::fromStdWString(val.toWString());
                res << QString::fromStdWString(val.toWString());
            }
            return res;
        }
        QString raw = QString::fromStdWString(values.front().toWString()).trimmed();
    
        // 启发式分隔
        raw.replace(" feat. ", ";", Qt::CaseInsensitive);
        raw.replace(" ft. ", ";", Qt::CaseInsensitive);
        raw.replace("、", ";");
        raw.replace("／", ";");
        raw.replace(";", ";");  // 防御性
    
        return raw.split(";", Qt::SkipEmptyParts)
                  .replaceInStrings(QRegularExpression("^\\s+|\\s+$"), "");
    }

    /**
     * @brief 获取音频专辑信息
     * @param errRes 获取失败返回的值
     * @return QString
     */
    QString getAlbum(QString&& errRes = "") const {
        if (mpegFile.isNull() || !mpegFile.tag()) {
            return errRes;
        }
        return QString::fromStdString(mpegFile.tag()->album().to8Bit(true));
    }

    /**
     * @brief 格式化音频时长为`HH:MM:SS`格式, 如`03:14`
     * @param errRes 获取失败返回的值
     * @return QString
     */
    QString formatTimeLengthToHHMMSS(QString&& errRes = "") const {
        if (mpegFile.isNull() || !mpegFile.audioProperties()) {
            return errRes;
        }
        if (auto sec = mpegFile.audioProperties()->lengthInSeconds();
            sec < 3600) {
            return QTime{0, 0}.addSecs(sec).toString("mm:ss");
        } else {
            return QTime{0, 0}.addSecs(sec).toString("hh:mm:ss");
        }
    }

    /**
     * @brief 获取音频的总秒数
     * @param errRes 获取失败返回的值
     * @return int
     */
    int getLengthInSeconds(int errRes = 0) const {
        if (mpegFile.isNull() || !mpegFile.audioProperties()) {
            return errRes;
        }
        return mpegFile.audioProperties()->lengthInSeconds();
    }

    /**
     * @brief 获取音频的总毫秒数
     * @param errRes 获取失败返回的值
     * @return int
     */
    int getLengthInMilliseconds(int errRes = 0) const {
        if (mpegFile.isNull() || !mpegFile.audioProperties()) {
            return errRes;
        }
        return mpegFile.audioProperties()->lengthInMilliseconds();
    }

    /**
     * @brief 尝试解析封面
     * @return std::optional<QPixmap> 
     */
    std::optional<QPixmap> getAlbumArtAdvanced() const {
        auto tryLoadImage = [](const TagLib::ByteVector& bv) -> std::optional<QPixmap> {
            QByteArray ba(bv.data(), static_cast<int>(bv.size()));
            QPixmap pixmap;
            if (pixmap.loadFromData(ba))
                return pixmap;
            return std::nullopt;
        };

        auto* baseFile = mpegFile.file();
        if (!baseFile) {
            return {};
        }

        // 1) MP3 / MP4 via FileRef complex props (APIC, covr atoms)
        TagLib::StringList keys;
        if (auto* tag = baseFile->tag()) {
            keys = tag->complexPropertyKeys();
        }

        for (const auto& key : keys) {
            auto props = baseFile->tag()->complexProperties(key);
            for (const auto& m : props) {
                for (const auto& p : m) {
                    if (p.second.type() == TagLib::Variant::ByteVector) {
                        auto bv = p.second.value<TagLib::ByteVector>();
                        if (auto res = tryLoadImage(bv)) {
                            return res;
                        }
                    }
                }
            }
        }

        // 2) FLAC / Ogg / Opus: Vorbis picture blocks
        if (auto* flac = dynamic_cast<TagLib::FLAC::File*>(baseFile)) {
            auto pics = flac->pictureList();
            if (!pics.isEmpty()) {
                auto& pic = pics.front();
                auto bv = pic->data();
                if (auto res = tryLoadImage(bv)) {
                    return res;
                }
            }
        }

        // @todo 3) 应该尝试寻找同目录下是否存在 Cover.jpg/png ...
        return {};
    }

    QString filePath() const {
        return _fileInfo.filePath();
    }

    QFileInfo const& fileInfo() const {
        return _fileInfo;
    }

private:
    QFileInfo const& _fileInfo;
    TagLib::FileRef mpegFile;
};

} // namespace HX

// 反射, 以便在 qml 中使用 (因为传参需要)
Q_DECLARE_METATYPE(HX::MusicInfo)

#endif // !_HX_MUSIC_INFO_H_
