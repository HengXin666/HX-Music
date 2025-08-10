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
#ifndef _HX_LYRIC_CONTROLLER_H_
#define _HX_LYRIC_CONTROLLER_H_

#include <QObject>
#include <QQuickImageProvider>
#include <QImage>
#include <QPainter>

#include <utils/AssParse.hpp>
#include <utils/SpscQueue.hpp>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>

#include <QDebug>

namespace HX {

namespace internal {

inline QByteArray readQrcFile(const QString& qrcPath) {
    QFile file(qrcPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray{};
    }
    return file.readAll(); // 读取整个文件内容
}

} // namespace internal

class LyricController : public QQuickImageProvider {
    Q_OBJECT
public:
    LyricController() 
        : QQuickImageProvider{QQuickImageProvider::Image}
        , _lastImage{}
        , _assParse{}
        , _assEffectParse{} 
    {
        _assParse.setFrameSize(1920, 1080);
        _assEffectParse.setFrameSize(1920, 1080);

        _assParse.readMemory(internal::readQrcFile(":default/default.ass"));
        _assEffectParse.readMemory(internal::readQrcFile(":default/default.ass"));

        /* musicPlayPosChanged (歌曲播放位置变化) */
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::musicPlayPosChanged,
            this,
            [this](qint64 pos) { updateCropFuckLyric(pos); });

        /* newSongLoaded (加载新歌) */
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::newSongLoaded,
            this,
            [this](HX::MusicInfo* info) { findLyricFile(*info); });
    }

    void findLyricFile(HX::MusicInfo const& info) {
        // 规则: 先查找 当前歌曲目录 是否存在 `歌曲名称.ass` 文件
        // 如果没有, 再查找 歌曲目录是否有`ass`文件夹, 进入重复上面的查找
        // todo 以后可以支持从缓存目录中查找, 如果没有就看看服务器?!
        std::filesystem::path musicPath{info.fileInfo().filesystemFilePath()};

        // 提取歌曲的文件名 (不包括扩展名)
        std::string songName = musicPath.stem().string();

        // 1. 先查找当前歌曲目录下是否有 `歌曲名称.ass` 文件
        std::filesystem::path lyricFilePath =
            musicPath.parent_path() / (songName + ".ass");
        qDebug() << "查找: " << lyricFilePath.c_str();
        if (std::filesystem::exists(lyricFilePath)) {
            qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.string().c_str();
            auto data = internal::separateAssFile(lyricFilePath);
            _assParse.readMemory(data.textAss.data());
            _assEffectParse.readMemory(data.nonTextAss.data());
            return;
        }

        // 2. 如果没有, 查找当前目录下是否有 `ass` 文件夹, 再查找歌词文件
        std::filesystem::path lyricFolderPath = musicPath.parent_path() / "ass";
        qDebug() << "查找: " << lyricFilePath.c_str();
        if (std::filesystem::exists(lyricFolderPath)
            && std::filesystem::is_directory(lyricFolderPath)) {
            lyricFilePath = lyricFolderPath / (songName + ".ass");
            if (std::filesystem::exists(lyricFilePath)) {
                qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.c_str();
                auto data = internal::separateAssFile(lyricFilePath);
                _assParse.readMemory(data.textAss.data());
                _assEffectParse.readMemory(data.nonTextAss.data());
                return;
            }
        }

        qWarning() << "[HX]: 没有找到歌词文件";
        // 使用默认的
        _assParse.readMemory(internal::readQrcFile(":default/default.ass"));
    }

void updateCropFuckLyric(qint64 nowTime) {
    constexpr int marginTopBottom = 10; // 上下额外边距
    constexpr int marginMiddle = 20;    // 上下字幕间的空白间距

    struct SubtitlePart {
        std::vector<ASS_Image*> images;
        int minY = INT_MAX;
        int maxY = 0;
    };

    int change01, change02;
    auto* imgList = _assParse.rendererFrame(nowTime + _offset, change01);
    auto* imgEffList = _assEffectParse.rendererFrame(nowTime + _offset, change02);
    if (!change01 && !change02)
        return;

    if (!imgList && !imgEffList) {
        if (!_lastImage.isNull()) {
            _lastImage = {1, 1, QImage::Format_ARGB32};
            _lastImage.fill(Qt::transparent);
            Q_EMIT updateLyriced();
        }
        return;
    }

    // 分组: 按 y 坐标判断上字幕和下字幕 (仅文本ASS)
    SubtitlePart top, bottom;
    int midLine = _assParse.getHeight() >> 1;
    for (ASS_Image* img = imgList; img; img = img->next) {
        if (img->dst_y < midLine) {
            top.images.push_back(img);
            top.minY = std::min(top.minY, img->dst_y);
            top.maxY = std::max(top.maxY, img->dst_y + img->h);
        } else {
            bottom.images.push_back(img);
            bottom.minY = std::min(bottom.minY, img->dst_y);
            bottom.maxY = std::max(bottom.maxY, img->dst_y + img->h);
        }
    }

    struct ImgInfo {
        ASS_Image* src = nullptr;
        QImage trimmedImage;
        int trimLeft = 0;
        int trimTop = 0;
        int visibleAbsLeft = 0;
        int visibleAbsTop = 0;
        int visibleW = 0;
        int visibleH = 0;
        int fullW = 0;
        int fullH = 0;
        int dstX = 0;
        int dstY = 0;
    };

    auto makeTrimmedInfo = [&](ASS_Image* img)->ImgInfo {
        ImgInfo info;
        info.src = img;
        info.fullW = img->w;
        info.fullH = img->h;
        info.dstX = img->dst_x;
        info.dstY = img->dst_y;

        QImage full(img->w, img->h, QImage::Format_RGBA8888);
        full.fill(Qt::transparent);
        uint8_t r = (img->color >> 24) & 0xFF;
        uint8_t g = (img->color >> 16) & 0xFF;
        uint8_t b = (img->color >> 8) & 0xFF;

        for (int y = 0; y < img->h; ++y) {
            uint8_t* srcLine = img->bitmap + y * img->stride;
            QRgb* destLine = reinterpret_cast<QRgb*>(full.scanLine(y));
            for (int x = 0; x < img->w; ++x) {
                destLine[x] = qRgba(r, g, b, srcLine[x]);
            }
        }

        int left = img->w, right = -1, topY = img->h, bottom = -1;
        for (int y = 0; y < img->h; ++y) {
            QRgb* destLine = reinterpret_cast<QRgb*>(full.scanLine(y));
            for (int x = 0; x < img->w; ++x) {
                if (qAlpha(destLine[x]) > 0) {
                    if (x < left) left = x;
                    if (x > right) right = x;
                    if (y < topY) topY = y;
                    if (y > bottom) bottom = y;
                }
            }
        }

        if (right < left || bottom < topY) {
            info.visibleW = info.visibleH = 0;
            info.visibleAbsLeft = img->dst_x;
            info.visibleAbsTop = img->dst_y;
            return info;
        }

        int w = right - left + 1;
        int h = bottom - topY + 1;
        info.trimLeft = left;
        info.trimTop = topY;
        info.visibleW = w;
        info.visibleH = h;
        info.visibleAbsLeft = img->dst_x + left;
        info.visibleAbsTop = img->dst_y + topY;
        info.trimmedImage = full.copy(left, topY, w, h);
        return info;
    };

    std::vector<ImgInfo> topInfos, bottomInfos, effInfos;
    for (auto* img : top.images) {
        auto inf = makeTrimmedInfo(img);
        if (inf.visibleW > 0) topInfos.push_back(std::move(inf));
    }
    for (auto* img : bottom.images) {
        auto inf = makeTrimmedInfo(img);
        if (inf.visibleW > 0) bottomInfos.push_back(std::move(inf));
    }
    for (ASS_Image* img = imgEffList; img; img = img->next) {
        auto inf = makeTrimmedInfo(img);
        if (inf.visibleW > 0) effInfos.push_back(std::move(inf));
    }

    if (topInfos.empty() && bottomInfos.empty() && effInfos.empty()) {
        if (!_lastImage.isNull()) {
            _lastImage = {1, 1, QImage::Format_ARGB32};
            _lastImage.fill(Qt::transparent);
            Q_EMIT updateLyriced();
        }
        return;
    }

    auto computeGroupBox = [](const std::vector<ImgInfo>& infos, int& minX, int& minY, int& maxX, int& maxY) {
        minX = INT_MAX; minY = INT_MAX; maxX = 0; maxY = 0;
        for (const auto& it : infos) {
            minX = std::min(minX, it.visibleAbsLeft);
            minY = std::min(minY, it.visibleAbsTop);
            maxX = std::max(maxX, it.visibleAbsLeft + it.visibleW);
            maxY = std::max(maxY, it.visibleAbsTop + it.visibleH);
        }
    };

    int topMinX, topMinY, topMaxX, topMaxY;
    int bottomMinX, bottomMinY, bottomMaxX, bottomMaxY;
    computeGroupBox(topInfos, topMinX, topMinY, topMaxX, topMaxY);
    computeGroupBox(bottomInfos, bottomMinX, bottomMinY, bottomMaxX, bottomMaxY);

    bool hasTop = !topInfos.empty();
    bool hasBottom = !bottomInfos.empty();

    int globalMinX = INT_MAX, globalMaxX = 0;
    int totalHeight = marginTopBottom * 2;
    int topHeight = 0, bottomHeight = 0;

    if (hasTop) {
        globalMinX = std::min(globalMinX, topMinX);
        globalMaxX = std::max(globalMaxX, topMaxX);
        topHeight = topMaxY - topMinY;
        totalHeight += topHeight;
    }
    if (hasBottom) {
        globalMinX = std::min(globalMinX, bottomMinX);
        globalMaxX = std::max(globalMaxX, bottomMaxX);
        bottomHeight = bottomMaxY - bottomMinY;
        if (hasTop) totalHeight += marginMiddle;
        totalHeight += bottomHeight;
    }

    // 宽度随特效动态变化
    for (const auto& inf : effInfos) {
        globalMinX = std::min(globalMinX, inf.visibleAbsLeft);
        globalMaxX = std::max(globalMaxX, inf.visibleAbsLeft + inf.visibleW);
    }

    if (globalMinX == INT_MAX) globalMinX = 0;
    int width = std::max(1, globalMaxX - globalMinX);
    int height = std::max(1, totalHeight);

    QImage result(width, height, QImage::Format_RGBA8888);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    int yOffset = marginTopBottom;
    if (hasTop) {
        for (const auto& info : topInfos) {
            painter.drawImage(info.visibleAbsLeft - globalMinX, (info.visibleAbsTop - topMinY) + yOffset, info.trimmedImage);
        }
        yOffset += topHeight;
    }
    if (hasTop && hasBottom) yOffset += marginMiddle;
    if (hasBottom) {
        for (const auto& info : bottomInfos) {
            painter.drawImage(info.visibleAbsLeft - globalMinX, (info.visibleAbsTop - bottomMinY) + yOffset, info.trimmedImage);
        }
    }

    // 绘制特效
    for (const auto& info : effInfos) {
        painter.drawImage(info.visibleAbsLeft - globalMinX, info.visibleAbsTop - topMinY + marginTopBottom, info.trimmedImage);
    }

    painter.end();
    _lastImage = std::move(result);
    Q_EMIT updateLyriced();
}


    void updateCropTwoLyric(qint64 nowTime) {
        struct SubtitlePart {
            std::vector<ASS_Image*> images;
            int minY = INT_MAX;
            int maxY = 0;
        };

        int change;
        auto* imgList = _assParse.rendererFrame(nowTime + _offset, change);
        if (!change)
            return;

        if (!imgList) {
            if (!_lastImage.isNull()) {
                _lastImage = {1, 1, QImage::Format_ARGB32};
                _lastImage.fill(Qt::transparent);
                Q_EMIT updateLyriced();
            }
            return;
        }

        // 分组: 按 y 坐标判断上字幕和下字幕
        SubtitlePart top, bottom;
        int midLine = _assParse.getHeight() >> 1; // 根据视频高度分界
        for (ASS_Image* img = imgList; img; img = img->next) {
            if (img->dst_y < midLine) {
                top.images.push_back(img);
                top.minY = std::min(top.minY, img->dst_y);
                top.maxY = std::max(top.maxY, img->dst_y + img->h);
            } else {
                bottom.images.push_back(img);
                bottom.minY = std::min(bottom.minY, img->dst_y);
                bottom.maxY = std::max(bottom.maxY, img->dst_y + img->h);
            }
        }

        // 用于存放裁剪后可见像素信息
        struct ImgInfo {
            ASS_Image* src = nullptr;
            QImage trimmedImage;
            int trimLeft = 0;
            int trimTop = 0;
            int visibleAbsLeft = 0;   // 在原始视频坐标系中的左上角
            int visibleAbsTop = 0;
            int visibleW = 0;
            int visibleH = 0;
            int fullW = 0;
            int fullH = 0;
            int dstX = 0;
            int dstY = 0;
        };

        auto makeTrimmedInfo = [&](ASS_Image* img)->ImgInfo {
            ImgInfo info;
            info.src = img;
            info.fullW = img->w;
            info.fullH = img->h;
            info.dstX = img->dst_x;
            info.dstY = img->dst_y;

            // 先构建完整 QImage
            QImage full(img->w, img->h, QImage::Format_RGBA8888);
            full.fill(Qt::transparent);
            uint8_t r = (img->color >> 24) & 0xFF;
            uint8_t g = (img->color >> 16) & 0xFF;
            uint8_t b = (img->color >> 8) & 0xFF;

            for (int y = 0; y < img->h; ++y) {
                uint8_t* srcLine = img->bitmap + y * img->stride;
                QRgb* destLine = reinterpret_cast<QRgb*>(full.scanLine(y));
                for (int x = 0; x < img->w; ++x) {
                    destLine[x] = qRgba(r, g, b, srcLine[x]);
                }
            }

            // 找到真实非透明区域
            int left = img->w, right = -1, topY = img->h, bottom = -1;
            for (int y = 0; y < img->h; ++y) {
                QRgb* destLine = reinterpret_cast<QRgb*>(full.scanLine(y));
                for (int x = 0; x < img->w; ++x) {
                    if (qAlpha(destLine[x]) > 0) {
                        if (x < left) left = x;
                        if (x > right) right = x;
                        if (y < topY) topY = y;
                        if (y > bottom) bottom = y;
                    }
                }
            }

            if (right < left || bottom < topY) {
                // 全透明, 返回空的 trimmedImage, visibleW/H = 0, 上层会跳过
                info.trimLeft = 0;
                info.trimTop = 0;
                info.visibleW = 0;
                info.visibleH = 0;
                info.visibleAbsLeft = img->dst_x;
                info.visibleAbsTop = img->dst_y;
                return info;
            }

            int w = right - left + 1;
            int h = bottom - topY + 1;
            info.trimLeft = left;
            info.trimTop = topY;
            info.visibleW = w;
            info.visibleH = h;
            info.visibleAbsLeft = img->dst_x + left;
            info.visibleAbsTop = img->dst_y + topY;
            info.trimmedImage = full.copy(left, topY, w, h);
            return info;
        };

        std::vector<ImgInfo> topInfos, bottomInfos;

        for (auto* img : top.images) {
            ImgInfo inf = makeTrimmedInfo(img);
            if (inf.visibleW > 0 && inf.visibleH > 0)
                topInfos.push_back(std::move(inf));
        }
        for (auto* img : bottom.images) {
            ImgInfo inf = makeTrimmedInfo(img);
            if (inf.visibleW > 0 && inf.visibleH > 0)
                bottomInfos.push_back(std::move(inf));
        }

        // 如果都没有可见像素, 清空并返回
        if (topInfos.empty() && bottomInfos.empty()) {
            if (!_lastImage.isNull()) {
                _lastImage = {1, 1, QImage::Format_ARGB32};
                _lastImage.fill(Qt::transparent);
                Q_EMIT updateLyriced();
            }
            return;
        }

        // 计算每组的可见包围盒(基于可见像素)
        auto computeGroupBox = [](const std::vector<ImgInfo>& infos, int& minX, int& minY, int& maxX, int& maxY) {
            minX = INT_MAX; minY = INT_MAX; maxX = 0; maxY = 0;
            for (const auto& it : infos) {
                minX = std::min(minX, it.visibleAbsLeft);
                minY = std::min(minY, it.visibleAbsTop);
                maxX = std::max(maxX, it.visibleAbsLeft + it.visibleW);
                maxY = std::max(maxY, it.visibleAbsTop + it.visibleH);
            }
            if (infos.empty()) {
                minX = minY = INT_MAX;
                maxX = maxY = 0;
            }
        };

        int topMinX, topMinY, topMaxX, topMaxY;
        int bottomMinX, bottomMinY, bottomMaxX, bottomMaxY;
        computeGroupBox(topInfos, topMinX, topMinY, topMaxX, topMaxY);
        computeGroupBox(bottomInfos, bottomMinX, bottomMinY, bottomMaxX, bottomMaxY);

        bool hasTop = !topInfos.empty();
        bool hasBottom = !bottomInfos.empty();

        int globalMinX = INT_MAX;
        int globalMaxX = 0;
        int totalHeight = 0;
        int topHeight = 0;
        int bottomHeight = 0;

        if (hasTop) {
            globalMinX = std::min(globalMinX, topMinX);
            globalMaxX = std::max(globalMaxX, topMaxX);
            topHeight = topMaxY - topMinY;
            totalHeight += topHeight;
        }
        if (hasBottom) {
            globalMinX = std::min(globalMinX, bottomMinX);
            globalMaxX = std::max(globalMaxX, bottomMaxX);
            bottomHeight = bottomMaxY - bottomMinY;
            totalHeight += bottomHeight;
        }

        if (globalMinX == INT_MAX) globalMinX = 0;
        int width = std::max(1, globalMaxX - globalMinX);
        int height = std::max(1, totalHeight);

        QImage result(width, height, QImage::Format_RGBA8888);
        result.fill(Qt::transparent);
        QPainter painter(&result);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // 画 top (从 y = 0 开始)
        painter.setPen(QPen(Qt::red, 1));
        for (const auto& info : topInfos) {
            int drawX = info.visibleAbsLeft - globalMinX;
            int drawY = info.visibleAbsTop - topMinY; // topMinY -> 0
            painter.drawImage(drawX, drawY, info.trimmedImage);

            // debug: 原始全图 bbox 用红, 裁剪后可见 bbox 用绿色
            painter.setPen(QPen(Qt::red, 1));
            painter.drawRect(info.dstX - globalMinX, info.dstY - topMinY, info.fullW - 1, info.fullH - 1);
            painter.setPen(QPen(Qt::green, 1));
            painter.drawRect(drawX, drawY, info.visibleW - 1, info.visibleH - 1);
        }

        // 画 bottom (紧接在 top 之后, 从 y = topHeight 开始)
        for (const auto& info : bottomInfos) {
            int drawX = info.visibleAbsLeft - globalMinX;
            int drawY = (info.visibleAbsTop - bottomMinY) + topHeight;
            painter.drawImage(drawX, drawY, info.trimmedImage);

            painter.setPen(QPen(Qt::red, 1));
            painter.drawRect(info.dstX - globalMinX, info.dstY - bottomMinY + topHeight, info.fullW - 1, info.fullH - 1);
            painter.setPen(QPen(Qt::green, 1));
            painter.drawRect(drawX, drawY, info.visibleW - 1, info.visibleH - 1);
        }
        painter.end();
        _lastImage = std::move(result);
        Q_EMIT updateLyriced();
    }


    void updateCropLyric(qint64 nowTime) {
        int change;
        auto* imgList = _assParse.rendererFrame(nowTime + _offset, change);

        if (!change) {
            return;
        }

        if (!imgList) {
            if (!_lastImage.isNull()) {
                // 清屏
                _lastImage = {1, 1, QImage::Format_ARGB32};
                _lastImage.fill(Qt::transparent);
                Q_EMIT updateLyriced();
            }
            return;
        }

        // 计算字幕图像的精确包围盒
        int minX = std::numeric_limits<int>::max();
        int minY = std::numeric_limits<int>::max();
        int maxX = 0, maxY = 0;

        for (ASS_Image* img = imgList; img; img = img->next) {
            minX = std::min(minX, img->dst_x);
            minY = std::min(minY, img->dst_y);
            maxX = std::max(maxX, img->dst_x + img->w);
            maxY = std::max(maxY, img->dst_y + img->h);
        }

        if (minX == std::numeric_limits<int>::max()) {
            // 没有有效图片
            return;
        }

        int width = maxX - minX;
        int height = maxY - minY;

        QImage result(width, height, QImage::Format_RGBA8888);
        result.fill(Qt::transparent);

        QPainter painter(&result);
        for (ASS_Image* img = imgList; img; img = img->next) {
            uint8_t r = (img->color >> 24) & 0xFF;
            uint8_t g = (img->color >> 16) & 0xFF;
            uint8_t b = (img->color >> 8) & 0xFF;

            QImage imgData(img->w, img->h, QImage::Format_RGBA8888);
            for (int y = 0; y < img->h; ++y) {
                uint8_t* srcLine = img->bitmap + y * img->stride;
                QRgb* destLine = reinterpret_cast<QRgb*>(imgData.scanLine(y));
                for (int x = 0; x < img->w; ++x) {
                    destLine[x] = qRgba(r, g, b, srcLine[x]);
                }
            }
            // 关键: 减去 minX 和 minY
            painter.drawImage(img->dst_x - minX, img->dst_y - minY, imgData);
        }
        painter.end();

        _lastImage = std::move(result);
        Q_EMIT updateLyriced();
    }

    void updateLyric(qint64 nowTime) {
        int change;
        auto* imgList = _assParse.rendererFrame(nowTime + _offset, change);

        if (!change) {
            return;
        }

        if (!imgList) {
            if (!_lastImage.isNull()) {
                // 清屏
                _lastImage = {1, 1, QImage::Format_ARGB32};
                _lastImage.fill(Qt::transparent);
                Q_EMIT updateLyriced();
            }
            return;
        }

        // 计算所有字幕图像的包围区域
        int width = 0, height = 0;
        for (ASS_Image* img = imgList; img; img = img->next) {
            width = std::max(width, img->dst_x + img->w);
            height = std::max(height, img->dst_y + img->h);
        }

        QImage result(width, height, QImage::Format_RGBA8888);
        result.fill(Qt::transparent);

        QPainter painter(&result);
        // 遍历 ASS_Image 链表
        for (ASS_Image* img = imgList; img; img = img->next) {
            uint8_t r = (img->color >> 24) & 0xFF;
            uint8_t g = (img->color >> 16) & 0xFF;
            uint8_t b = (img->color >> 8) & 0xFF;
            // uint8_t a = (img->color >>  0) & 0xFF;

            // 创建临时 QImage 存放当前字幕图像
            QImage imgData(img->w, img->h, QImage::Format_RGBA8888);
            for (int y = 0; y < img->h; ++y) {
                uint8_t* srcLine = img->bitmap + y * img->stride;
                QRgb* destLine = reinterpret_cast<QRgb*>(imgData.scanLine(y));
                for (int x = 0; x < img->w; ++x) {
                    // uint8_t alpha = 255 - srcLine[x]; // 反转 alpha 值
                    destLine[x] = qRgba(r, g, b, srcLine[x]);
                }
            }
            painter.drawImage(img->dst_x, img->dst_y, imgData);
        }
        painter.end();

        _lastImage = std::move(result);
        Q_EMIT updateLyriced(); // 发送渲染完成信号
    }

    // QML 调用
    QImage requestImage(
        [[maybe_unused]] QString const& id,
        [[maybe_unused]] QSize* size,
        [[maybe_unused]] QSize const& requestedSize) override {
        // 取最新一帧, 如果没有就返回上一帧的副本
        if (size) {
            *size = _lastImage.size();
        }
        return _lastImage;
    }

Q_SIGNALS:
    void updateLyriced();

private:
    QImage _lastImage;
    AssParse _assParse;
    AssParse _assEffectParse;
    long long _offset = 0; // 字幕偏移量
};

} // namespace HX

#endif // !_HX_LYRIC_CONTROLLER_H_