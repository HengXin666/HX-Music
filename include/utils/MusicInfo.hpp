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
#include <taglib/tag.h>
#include <taglib/audioproperties.h>

#include <taglib/mpegfile.h>

#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>

#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4item.h>
#include <taglib/mp4coverart.h>

#include <taglib/asffile.h>
#include <taglib/asftag.h>

#include <taglib/rifffile.h>
#include <taglib/wavfile.h>
#include <taglib/oggflacfile.h>

#include <QTime>
#include <QFileInfo>
#include <QPixmap>
#include <QString>
#include <QSet>

#include <utils/FileInfo.hpp>

namespace HX {

namespace internal {

#if 0
/**
 * @brief 支持的文件类型
 */
inline const QSet<QString> extensionSet{
    "mp3",
    "wav",
    "flac",
    "ogg",
    "mpc",
    "spx",
    "wv",
    "tta",
    "aiff",
    "aif",
    "mp4",
    "ape",
    "asf",
    "dsf",
    "dff",
    "acc",
};
#endif

} // namespace internal

class MusicInfo : public QObject {
    Q_OBJECT
public:
    explicit MusicInfo(QFileInfo const& fileInfo) 
        : _fileInfo(fileInfo)
        , _byteArr(QFile::encodeName(_fileInfo.canonicalFilePath()))
        , mpegFile(_byteArr.constData())
    {}

#if 0 /// @note 不支持你也没辙, 而且后缀可能是假的, 得看二进制
    /**
     * @brief 是否不支持该格式
     * @param fileInfo
     * @return true 是不支持
     * @return false 不是不支持
     */
    inline static bool isNotSupport(QFileInfo const& fileInfo) {
        return internal::extensionSet.find(fileInfo.suffix().toLower())
               == internal::extensionSet.end();
    }
#endif

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

    // 高性能获取专辑图片函数, 支持 MP3、FLAC、MP4/M4A 格式
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

        /*
                // 1. MP3 或 AAC(ID3v2)
                if (ext == "mp3" || ext == "aac") {
                    TagLib::MPEG::File file(_byteArr.constData(),
           _byteArr.size()); TagLib::ID3v2::Tag* tag = file.ID3v2Tag(true); if
           (tag) { TagLib::ID3v2::FrameList frames = tag->frameList("APIC"); if
           (frames.isEmpty()) frames = tag->frameList("PIC"); // 兼容旧版
           ID3v2.2 if (!frames.isEmpty()) { auto* picFrame =
                                dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(
                                    frames.front());
                            if (picFrame)
                                return tryLoadImage(picFrame->picture());
                        }
                    }
                }

                // 2. FLAC
                else if (ext == "flac") {
                    // https://taglib.org/api/classTagLib_1_1FLAC_1_1File.html
                    TagLib::FLAC::File file(_byteArr.constData(),
           _byteArr.size()); if (!file.isValid()) return std::nullopt;

                    // 策略1: 标准PICTURE blocks
                    const auto& pictures = file.pictureList();
                    for (const auto* pic : pictures) {
                        if (!pic)
                            continue;
                        if (auto res = tryLoadImage(pic->data())) {
                            return res;
                        }
                    }

                    // 策略2: XiphComment元数据块
                    if (file.hasXiphComment()) {
                        if (auto* xiph = file.xiphComment()) {
                            // METADATA_BLOCK_PICTURE 字段, Base64编码
                            const auto& metaPics =
                                xiph->fieldListMap().value("METADATA_BLOCK_PICTURE");
                            for (const auto& str : metaPics) {
                                // 不使用 QString 转换, 直接用 to8Bit() 转
           QByteArray, 再
                                // Base64 解码
                                QByteArray base64Data = str.to8Bit().data();
                                QByteArray decoded =
           QByteArray::fromBase64(base64Data);

                                TagLib::FLAC::Picture pic;
                                if (pic.parse(
                                        TagLib::ByteVector(
                                            decoded.constData(),
           decoded.size()))) { if (auto result = tryLoadImage(pic.data());
           result) { qDebug()
                                            << "FLAC: Found cover in
           METADATA_BLOCK_PICTURE"; return result;
                                    }
                                }
                            }

                            // COVERART 字段, Base64编码的纯图片数据
                            const auto& coverArts =
           xiph->fieldListMap().value("COVERART"); for (const auto& str :
           coverArts) { QByteArray base64Data = str.to8Bit().data(); QByteArray
           decoded = QByteArray::fromBase64(base64Data);

                                TagLib::ByteVector picData(
                                    decoded.constData(), decoded.size());
                                if (auto result = tryLoadImage(picData); result)
           { qDebug() << "FLAC: Found cover in COVERART field"; return result;
                                }
                            }
                        } else {
                            auto& list = file.complexPropertyKeys();
                        }
                    }

                    // 策略3: ID3v2 标签
                    if (file.hasID3v2Tag()) {
                        if (auto* tag = file.ID3v2Tag()) {
                            auto frames = tag->frameList("APIC");
                            if (frames.isEmpty())
                                frames = tag->frameList("PIC");

                            for (auto* frame : frames) {
                                if (auto* picFrame = dynamic_cast<
                                        TagLib::ID3v2::AttachedPictureFrame*>(frame))
           { if (auto result = tryLoadImage(picFrame->picture()); result) {
                                        qDebug() << "FLAC: Found cover in ID3v2
           tag"; return result;
                                    }
                                }
                            }
                        }
                    }

                    // 策略4: ASF封面
                    TagLib::ASF::File asfFile(_byteArr.constData(),
           _byteArr.size()); TagLib::ASF::Tag* tag = asfFile.tag(); if (tag) {
                        const TagLib::ASF::AttributeListMap& attrListMap =
                            tag->attributeListMap();
                        auto it = attrListMap.find("WM/Picture");
                        if (it != attrListMap.end() && !it->second.isEmpty()) {
                            const TagLib::ASF::Picture pic =
                                it->second.front().toPicture();
                            return tryLoadImage(pic.picture());
                        }
                    }
                }
                // 3. MP4 / M4A
                else if (ext == "mp4" || ext == "m4a") {
                    TagLib::MP4::File file(_byteArr.constData(),
           _byteArr.size()); if (file.isValid() && file.tag()) { auto items =
           file.tag()->itemMap(); if (items.contains("covr")) {
                            TagLib::MP4::CoverArtList covers =
                                items["covr"].toCoverArtList();
                            if (!covers.isEmpty())
                                return tryLoadImage(covers.front().data());
                        }
                    }
                }

                // 4. WAV (可能内嵌 ID3v2)
                else if (ext == "wav") {
                    TagLib::RIFF::WAV::File file(_byteArr.constData(),
           _byteArr.size()); TagLib::ID3v2::Tag* tag = file.ID3v2Tag(); if (tag)
           { TagLib::ID3v2::FrameList frames = tag->frameList("APIC"); if
           (frames.isEmpty()) frames = tag->frameList("PIC"); if
           (!frames.isEmpty()) { auto* picFrame =
                                dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(
                                    frames.front());
                            if (picFrame)
                                return tryLoadImage(picFrame->picture());
                        }
                    }
                }
                // 5. WMA / ASF (WM/Picture)
                else if (ext == "wma" || ext == "asf") {
                    TagLib::ASF::File file(_byteArr.constData(),
           _byteArr.size()); if (file.isValid() && file.tag()) { auto attrMap =
           file.tag()->attributeListMap(); if (attrMap.contains("WM/Picture")) {
                            const auto& list = attrMap["WM/Picture"];
                            if (!list.isEmpty()) {
                                TagLib::ASF::Picture pic =
           list.front().toPicture(); if (pic.isValid()) return
           tryLoadImage(pic.picture());
                            }
                        }
                    }
                }

                // 未知或未支持格式, 或无嵌入封面
                qDebug() << "解析失败:" << _byteArr;
        */
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
    QByteArray _byteArr;
    TagLib::FileRef mpegFile;
};

} // namespace HX

// 反射, 以便在 qml 中使用
Q_DECLARE_METATYPE(HX::MusicInfo)

#endif // !_HX_MUSIC_INFO_H_
