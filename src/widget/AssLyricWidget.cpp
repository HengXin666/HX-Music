#include <widget/AssLyricWidget.h>

#include <filesystem>

#include <QPainter>
#include <QResizeEvent>

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>

#include <QTime>

QByteArray readQrcFile(const QString& qrcPath) {
    QFile file(qrcPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray{};
    }
    return file.readAll(); // 读取整个文件内容
}

AssLyricWidget::AssLyricWidget(QWidget* parent)
    : QWidget(parent)
    , _assParse()
{
    resize(1920, 1080);
    setMinimumSize(1920 / 6, 1080 / 6);
    _assParse.setFrameSize(1920, 1080);

    if (auto it = GlobalSingleton::get().playQueue.now()) {
        findLyricFile(
            HX::MusicInfo { QFileInfo {
                (*it)->getData()
            }}
        );
    } else {
        _assParse.readMemory(readQrcFile(":default/default.ass"));
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

void AssLyricWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    // painter.setRenderHint(QPainter::HighQualityAntialiasing);

    if (_isMove) {
        painter.setBrush(QColor(99,0,99, 10));
        painter.drawRect(0, 0, width(), height()); // 先画成黑色
    }

    if (_img.isNull()) {
        return;
    }
    
    // 渲染字幕
    painter.drawImage(0, 0, _img);
}

void AssLyricWidget::resizeEvent(QResizeEvent* event) {
    auto [w, h] = event->size();
    _assParse.setFrameSize(w, h);
    updateLyric(GlobalSingleton::get().music.getNowPos());
}

void AssLyricWidget::findLyricFile(HX::MusicInfo const& info) {
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
        qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.c_str();
        _assParse.readFile(lyricFilePath.string().c_str());
        return;
    }

    // 2. 如果没有，查找当前目录下是否有 `ass` 文件夹，再查找歌词文件
    std::filesystem::path lyricFolderPath = musicPath.parent_path() / "ass";
    qDebug() << "查找: " << lyricFilePath.c_str();
    if (std::filesystem::exists(lyricFolderPath) && std::filesystem::is_directory(lyricFolderPath)) {
        lyricFilePath = lyricFolderPath / (songName + ".ass");
        if (std::filesystem::exists(lyricFilePath)) {
            qInfo() << "[HX]: 找到歌词文件: " << lyricFilePath.c_str();
            _assParse.readFile(lyricFilePath.string().c_str());
            return;
        }
    }

    qWarning() << "[HX]: 没有找到歌词文件";
    // 使用默认的
    _assParse.readMemory(readQrcFile(":default/default.ass"));
}

void AssLyricWidget::updateLyric(qint64 nowTime) {
    int change;
    auto* imgList = _assParse.rendererFrame(nowTime + _offset, change);

    if (!change) {
        return;
    }
    
    if (!imgList) {
        if (!_img.isNull()) {
            // 清屏
            _img = {};
            update();
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
        // 将处理后的字幕图像绘制到目标 QImage 上
        painter.drawImage(img->dst_x, img->dst_y, imgData);
    }

    painter.end();
    _img = result;
    update();
}
