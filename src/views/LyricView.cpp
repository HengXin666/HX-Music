#include <views/LyricView.h>

#include <QPainter>
#include <QResizeEvent>

#include <singleton/SignalBusSingleton.h>

LyricView::LyricView(QWidget* parent)
    : QWidget(parent)
    , _assParse()
{
    _assParse.setFrameSize(width(), height());

    _assParse.readFile("/run/media/loli/アニメ専門/音乐/ass/命运石之门 0 OP1.ass");

    connect(&SignalBusSingleton::get(), &SignalBusSingleton::musicPlayPosChanged, this,
        [this](qint64 pos){
        updateSubtitle(pos);
    });
}

void LyricView::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    // painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setBrush(QColor(0,0,0));
    painter.drawRect(0,0,this->width(),this->height()); //先画成黑色


    if (_img.isNull()) {
        return;
    }
    
    // 渲染字幕
    painter.drawImage(0, 0, _img);
}

void LyricView::resizeEvent(QResizeEvent* event) {
    auto [w, h] = event->size();
    _assParse.setFrameSize(w, h);
}

void LyricView::updateSubtitle(qint64 nowTime) {
    int change;
    auto* imgList = _assParse.rendererFrame(nowTime, change);
    if (!imgList || !change) {
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