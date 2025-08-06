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
    {
        _assParse.setFrameSize(1920, 1080);

        if (auto uri = GlobalSingleton::get().playQueue.now()) {
            findLyricFile(HX::MusicInfo{QFileInfo{*uri}});
        } else {
            _assParse.readMemory(internal::readQrcFile(":default/default.ass"));
        }

        /* musicPlayPosChanged (歌曲播放位置变化) */
        connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicPlayPosChanged, this,
            [this](qint64 pos) {
            updateLyric(pos);
        });

        /* newSongLoaded (加载新歌) */
        connect(&SignalBusSingleton::get(), &SignalBusSingleton::newSongLoaded, this,
            [this](HX::MusicInfo const& info) {
            findLyricFile(info);
        });
    }

    void findLyricFile(HX::MusicInfo const& info) {
        // 规则: 先查找 当前歌曲目录 是否存在 `歌曲名称.ass` 文件
        // 如果没有, 再查找 歌曲目录是否有`ass`文件夹, 进入重复上面的查找
        // todo 以后可以支持从缓存目录中查找, 如果没有就看看服务器?!
        std::filesystem::path musicPath{info.fileInfo().filesystemFilePath()};
        
        // 提取歌曲的文件名 (不包括扩展名)
        std::string songName = musicPath.stem().string();

        // 1. 先查找当前歌曲目录下是否有 `歌曲名称.ass` 文件
        std::filesystem::path lyricFilePath = musicPath.parent_path() / (songName + ".ass");
        qDebug() << "查找: " << lyricFilePath.c_str();
        if (std::filesystem::exists(lyricFilePath)) {
            qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.string().c_str();
            _assParse.readFile(lyricFilePath.c_str());
            return;
        }

        // 2. 如果没有，查找当前目录下是否有 `ass` 文件夹，再查找歌词文件
        std::filesystem::path lyricFolderPath = musicPath.parent_path() / "ass";
        qDebug() << "查找: " << lyricFilePath.c_str();
        if (std::filesystem::exists(lyricFolderPath) && std::filesystem::is_directory(lyricFolderPath)) {
            lyricFilePath = lyricFolderPath / (songName + ".ass");
            if (std::filesystem::exists(lyricFilePath)) {
                qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.c_str();
                _assParse.readFile(lyricFilePath.c_str());
                return;
            }
        }

        qWarning() << "[HX]: 没有找到歌词文件";
        // 使用默认的
        _assParse.readMemory(internal::readQrcFile(":default/default.ass"));
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
                _lastImage = {};
                emit updateLyriced();
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
            uint8_t b = (img->color >>  8) & 0xFF;
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
        emit updateLyriced(); // 发送渲染完成信号
    }

    Q_INVOKABLE void prev() {
        qDebug() << "上一首";
    }
    Q_INVOKABLE void next() {
        qDebug() << "下一首";
    }
    Q_INVOKABLE void togglePause() { 
        /* 播放/暂停 */
    }

    // QML 调用
    QImage requestImage(
        [[maybe_unused]] QString const& id,
        [[maybe_unused]] QSize* size,
        [[maybe_unused]] QSize const& requestedSize
    ) override {
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
    long long _offset = 0;  // 字幕偏移量
};

} // namespace HX

#endif // !_HX_LYRIC_CONTROLLER_H_