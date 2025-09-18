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
#include <QApplication>
#include <QQuickImageProvider>
#include <QImage>
#include <QPainter>
#include <QDebug>

#include <utils/AssParse.hpp>
#include <utils/SpscQueue.hpp>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.h>
#include <controller/MessageController.h>
#include <config/LyricConfig.hpp>
#include <api/LyricsApi.hpp>

#include <HXLibs/utils/FileUtils.hpp>
#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>
#include <HXLibs/log/Log.hpp>

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

    struct TwoBlockBounds {
        int topYMin = std::numeric_limits<int>::max();
        int topYMax = 0;
        int btmYMin = std::numeric_limits<int>::max();
        int btmYMax = 0;
        int left = std::numeric_limits<int>::max();
        int right = 0;

        bool hasTop() const noexcept {
            return topYMin != std::numeric_limits<int>::max();
        }

        bool hasBtm() const noexcept {
            return btmYMin != std::numeric_limits<int>::max();
        }
    };
public:
    LyricController()
        : QQuickImageProvider{QQuickImageProvider::Image}
        , _lastImage{}
        , _assParse{}
    {
        _assParse.setFrameSize(1920, 1080);
        {
            auto buf = internal::readQrcFile(":default/default.ass");
            _assParse.readMemory({buf.data(), static_cast<std::size_t>(buf.size())});
        }

        // 加载配置文件
        coroutine::EventLoop loop{};
        utils::AsyncFile file{loop};
        try {
            file.syncOpen("./lyricConfig.json", utils::OpenMode::Read);
            reflection::fromJson(_lyricConfig, file.syncReadAll());
            file.syncClose();
        } catch (...) {
            _lyricConfig = {
                400, 300,
                800, 200,
                0, false,
                false, false,
                true
            };
        }

        /* musicPlayPosChanged (歌曲播放位置变化) */
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::musicPlayPosChanged,
            this,
            [this](qint64 pos) {
                if (_lyricConfig.isFullScreen) {
                    renderLyricByFullScreen(pos);
                } else {
                    renderLyric(pos);
                }
            });

        /* newSongLoaded (加载新歌) */
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::newSongLoaded,
            this,
            [this](MusicInformation* info) {
                if (GlobalSingleton::get().musicConfig.playlistId == Playlist::kNonePlaylist) {
                    // auto path = findLyricFile(info->filePath());
                    // auto data = utils::FileUtils::getFileContent(path);
                    // _assParse = preprocessLyricBoundingBoxes(
                    //     0,
                    //     info->getLengthInMilliseconds(),
                    //     data
                    // );
                    MessageController::get().show<MsgType::Warning>("ub: 歌词本地加载 @todo");
                } else {
                    {
                        auto buf = internal::readQrcFile(":default/loading.ass");
                        _assParse.readMemory({buf.data(), static_cast<std::size_t>(buf.size())});
                    }
                    renderAFrameInstantly();
                    log::hxLog.debug("加载歌词:", info->getId());
                    // 网络加载歌词到内存
                    LyricsApi::getAssLyrics(info->getId())
                        .thenTry([this, ms = info->getLengthInMilliseconds()](container::Try<std::string> t) {
                            if (!t) [[unlikely]] {
                                MessageController::get().show<MsgType::Error>("加载歌词失败:" + t.what());
                                return;
                            }
                            QMetaObject::invokeMethod(
                                QCoreApplication::instance(),
                                [this, ms, buf = t.move()]{
                                _assParse = preprocessLyricBoundingBoxes(
                                    0,
                                    ms,
                                    buf
                                );
                                renderAFrameInstantly();
                            });
                        });
                }
            });

        /* lyricAddOffset 歌词加上偏移量 */
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::lyricAddOffset,
            this,
            [this](long long add) {
                _lyricConfig.lyricOffset += add;
                saveConfig();
            });

        /* isFullScreenChanged 切换全屏, 需要立即渲染新模式的一帧 */
        connect(this, &LyricController::isFullScreenChanged, this, [this] {
            renderAFrameInstantly();
        });
    }

    /**
     * @brief 保存配置文件
     */
    void saveConfig() noexcept {
        // 保存配置文件
        coroutine::EventLoop loop{};
        utils::AsyncFile file{loop};
        file.syncOpen("./lyricConfig.json", utils::OpenMode::Write);
        std::string json;
        reflection::toJson<true>(_lyricConfig, json);
        file.syncWrite(json);
        file.syncClose();
    }

    decltype(std::declval<std::filesystem::path>().string()) findLyricFile(
        QString path
    ) {
        // 规则: 先查找 当前歌曲目录 是否存在 `歌曲名称.ass` 文件
        // 如果没有, 再查找 歌曲目录是否有`ass`文件夹, 进入重复上面的查找
        // todo 以后可以支持从缓存目录中查找, 如果没有就看看服务器?!
        std::filesystem::path musicPath{path.toStdString()};

        // 提取歌曲的文件名 (不包括扩展名)
        std::string songName = musicPath.stem().string();

        // 1. 先查找当前歌曲目录下是否有 `歌曲名称.ass` 文件
        std::filesystem::path lyricFilePath =
            musicPath.parent_path() / (songName + ".ass");
        qDebug() << "查找: " << lyricFilePath.c_str();
        if (std::filesystem::exists(lyricFilePath)) {
            qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.string().c_str();
            return lyricFilePath.string();
        }

        // 2. 如果没有, 查找当前目录下是否有 `ass` 文件夹, 再查找歌词文件
        std::filesystem::path lyricFolderPath = musicPath.parent_path() / "ass";
        qDebug() << "查找: " << lyricFilePath.c_str();
        if (std::filesystem::exists(lyricFolderPath)
            && std::filesystem::is_directory(lyricFolderPath)) {
            lyricFilePath = lyricFolderPath / (songName + ".ass");
            if (std::filesystem::exists(lyricFilePath)) {
                qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.c_str();
                return lyricFilePath.string();
            }
        }

        qWarning() << "[HX]: 没有找到歌词文件";
        // 使用默认的
        return decltype(lyricFilePath.string()){};
    }

    // 并行计算-缓存Ass范围用于切割
    AssParse preprocessLyricBoundingBoxes(qint64 startTime, qint64 endTime, std::string_view data);

    // 辅助函数: 绘制ASS图像到指定画布
    inline void drawAssImage(QPainter* painter, ASS_Image* img, int baseX, int baseY) {
        // 更正: Ass 使用的颜色是 BGR, 而不是 RGB
        uint8_t b = (img->color >> 24) & 0xFF;
        uint8_t g = (img->color >> 16) & 0xFF;
        uint8_t r = (img->color >> 8) & 0xFF;
        // uint8_t a = (img->color >>  0) & 0xFF;

        QImage image(img->w, img->h, QImage::Format_RGBA8888);
        for (int y = 0; y < img->h; ++y) {
            uint8_t* srcLine = img->bitmap + y * img->stride;
            QRgb* destLine = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < img->w; ++x) {
                // uint8_t alpha = 255 - srcLine[x]; // 反转 alpha 值
                destLine[x] = qRgba(r, g, b, srcLine[x]);
            }
        }
        painter->drawImage(img->dst_x - baseX, img->dst_y - baseY, image);
    }

    inline void drawAssImage(QPainter* painter, ASS_Image* img, bool isTop) {
        // 更正: Ass 使用的颜色是 BGR, 而不是 RGB
        uint8_t b = (img->color >> 24) & 0xFF;
        uint8_t g = (img->color >> 16) & 0xFF;
        uint8_t r = (img->color >> 8) & 0xFF;
        // uint8_t a = (img->color >>  0) & 0xFF;

        QImage image{img->w, img->h, QImage::Format_RGBA8888};
        for (int y = 0; y < img->h; ++y) {
            uint8_t* srcLine = img->bitmap + y * img->stride;
            QRgb* destLine = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < img->w; ++x) {
                // uint8_t alpha = 255 - srcLine[x]; // 反转 alpha 值
                destLine[x] = qRgba(r, g, b, srcLine[x]);
            }
        }
        if (isTop) {
            painter->drawImage(
                img->dst_x - _twoBlockBounds.left,
                img->dst_y - _twoBlockBounds.topYMin,
                image
            );
        } else {
            painter->drawImage(
                img->dst_x - _twoBlockBounds.left,
                img->dst_y - _twoBlockBounds.btmYMin
                + (_twoBlockBounds.btmYMax - _twoBlockBounds.btmYMin),
                image
            );
        }
    }

    // 2. 渲染: 使用预计算的边界框
    void renderLyric(qint64 nowTime, bool mustBeUpdated = false) {
        if (!_hasCachedBlock) [[unlikely]] {
            return;
        }

        int change;
        auto* imgList = _assParse.rendererFrame(nowTime + _lyricConfig.lyricOffset, change);
        if (!imgList) {
            if (!_lastImage.isNull()) {
                _lastImage = QImage(1, 1, QImage::Format_ARGB32);
                _lastImage.fill(Qt::transparent);
                Q_EMIT updateLyriced();
            }
            return;
        }
        if (!change && !mustBeUpdated) {
            return;
        }

        // 按上下分组
        int midLine = _assParse.getHeight() >> 1;
        int w = _twoBlockBounds.right - _twoBlockBounds.left;
        int h = (_twoBlockBounds.btmYMax - _twoBlockBounds.btmYMin)
              + (_twoBlockBounds.topYMax - _twoBlockBounds.topYMin);
        QImage res{w, h, QImage::Format_RGBA8888};
        res.fill(Qt::transparent);
        QPainter painter(&res);
        // 自动居中
        int left = std::numeric_limits<int>::max();
        int right = 0;
        for (ASS_Image* img = imgList; img; img = img->next) {
            left = std::min(left, img->dst_x);
            right = std::max(right, img->dst_x + img->w);
        }

        // 计算水平偏移量
        int offsetX = (w - (right - left)) >> 1;

        // 先平移, 再绘制
        if (_lyricConfig.isAutoCenter && offsetX > 0 && offsetX + right <= w) [[likely]] {
            painter.translate(offsetX, 0);
        }

        for (ASS_Image* img = imgList; img; img = img->next) {
            drawAssImage(&painter, img, img->dst_y + (img->h >> 1) < midLine);
        }

        // 别问, 问就是 preprocessLyricBoundingBoxes 有bug, 无法准确计算 right ...
        // 间歇性出现...
        _twoBlockBounds.left = std::min(left, _twoBlockBounds.left);
        _twoBlockBounds.right = std::max(right, _twoBlockBounds.right);
        _lastImage = std::move(res);
        Q_EMIT updateLyriced();
    }

    // 全屏渲染, 原样展示
    void renderLyricByFullScreen(qint64 nowTime, bool mustBeUpdated = false) {
        int change;
        auto* imgList = _assParse.rendererFrame(nowTime + _lyricConfig.lyricOffset, change);
        if (!imgList) {
            if (!_lastImage.isNull()) {
                _lastImage = QImage(1, 1, QImage::Format_ARGB32);
                _lastImage.fill(Qt::transparent);
                Q_EMIT updateLyriced();
            }
            return;
        }
        if (!change && !mustBeUpdated) {
            return;
        }

        // 创建最终画布
        QImage result(_assParse.getWidth(), _assParse.getHeight(), QImage::Format_ARGB32);
        result.fill(Qt::transparent);
        QPainter painter(&result);
        for (ASS_Image* img = imgList; img; img = img->next) {
            drawAssImage(&painter, img, 0, 0);
        }

        _lastImage = std::move(result);
        Q_EMIT updateLyriced();
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
    
    // 立即渲染一帧
    Q_INVOKABLE void renderAFrameInstantly() {
        if (_lyricConfig.isFullScreen) {
            renderLyricByFullScreen(GlobalSingleton::get().music.getNowPos(), true);
        } else {
            renderLyric(GlobalSingleton::get().music.getNowPos(), true);
        }
    }

    // 爬取歌曲歌词
    Q_INVOKABLE void crawlKaRaOKAssLyrics(uint64_t id) {
        LyricsApi::crawlKaRaOKAssLyrics(id)
            .thenTry([=](auto t) {
                if (!t) [[unlikely]] {
                    MessageController::get().show<MsgType::Error>("爬取歌词失败:" + t.what());
                    return;
                }
                MessageController::get().show<MsgType::Info>("成功获取到歌词!" + std::to_string(id));
            });
    }
Q_SIGNALS:
    void updateLyriced();

private:
    QImage _lastImage;
    AssParse _assParse;
    LyricConfig _lyricConfig;
    TwoBlockBounds _twoBlockBounds;
    bool _hasCachedBlock = false;       // 是否计算了边界块

#ifndef Q_MOC_RUN
    #define HX_QML_CONFIG_TYPE(name) decltype(_lyricConfig.name)
#else
    #define HX_QML_CONFIG_TYPE(name) int
#endif

/**
 * @brief 快速注册为 QML 可以用成员
 * @param name 外露的成员名称, 内部为 _config.name
 */
#define HX_QML_CONFIG_PROPERTY(name)                                           \
private:                                                                       \
    Q_PROPERTY(HX_QML_CONFIG_TYPE(name)                                        \
                   name READ name WRITE set##name NOTIFY name##Changed)        \
public:                                                                        \
    HX_QML_CONFIG_TYPE(name) name() const noexcept {                           \
        return _lyricConfig.name;                                              \
    }                                                                          \
    void set##name(HX_QML_CONFIG_TYPE(name) const& name) noexcept {            \
        if (_lyricConfig.name != name) {                                       \
            _lyricConfig.name = name;                                          \
            Q_EMIT name##Changed();                                            \
        }                                                                      \
    }                                                                          \
Q_SIGNALS:                                                                     \
    void name##Changed()

    HX_QML_CONFIG_PROPERTY(windowX);
    HX_QML_CONFIG_PROPERTY(windowY);
    HX_QML_CONFIG_PROPERTY(windowWidth);
    HX_QML_CONFIG_PROPERTY(windowHeight);
    HX_QML_CONFIG_PROPERTY(maeWindowX);
    HX_QML_CONFIG_PROPERTY(maeWindowY);
    HX_QML_CONFIG_PROPERTY(maeWindowWidth);
    HX_QML_CONFIG_PROPERTY(maeWindowHeight);
    HX_QML_CONFIG_PROPERTY(lyricOffset);
    HX_QML_CONFIG_PROPERTY(isWindowOpened);
    HX_QML_CONFIG_PROPERTY(isLocked);
    HX_QML_CONFIG_PROPERTY(isFullScreen);
    HX_QML_CONFIG_PROPERTY(isAutoCenter);

#undef HX_QML_CONFIG_PROPERTY
};


} // namespace HX

#endif // !_HX_LYRIC_CONTROLLER_H_