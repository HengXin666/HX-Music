#include <widget/ScrollText.h>

#include <QPainter>
#include <QTimer>
#include <QTimerEvent>

ScrollText::ScrollText(QWidget* parent)
    : QLabel(parent)
{}

void ScrollText::setText(const QString& text) {
    _text = text;
    _offset = 0;
    update();
    updateGeometry();
    int textWidth = fontMetrics().horizontalAdvance(_text);
    if (textWidth >= width()) {
        _timerState = TimerState::Open;
        openTimer();
    } else {
        _timerState = TimerState::Close;
        closeTimer();
    }
}

void ScrollText::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    int textWidth = fontMetrics().horizontalAdvance(_text);

    int y = height() / 2 + fontMetrics().ascent() / 2 - 2;  // 使文本垂直居中
    int x = -_offset;
    
    // 先绘制第一个部分
    painter.drawText(x, y, _text);
    
    // 如果文本完全滚动出去, 则从头开始绘制
    if (_timerState == TimerState::Open && x + textWidth < width()) {
        painter.drawText(x + textWidth + _gap, y, _text);
    }
}

void ScrollText::timerEvent(QTimerEvent* event) {
    if (event->timerId() == _timerId) {
        _offset += 1; // 让文字向左滚动
        
        // 如果滚动到文本的结束部分, 重置偏移量
        if (_offset >= _gap + fontMetrics().horizontalAdvance(_text)) {
            _offset = 0;
            // 如果存在间隔时间, 则暂停滚动
            if (_pauseTime) {
                killTimer(_timerId);
                _timerId = 0;
                QTimer::singleShot(_pauseTime, this,
                    [this]() {
                    _timerId = startTimer(_updateTime);
                });
            }
        }
        update(); // 强制重绘, 保证滚动效果
    } else {
        QWidget::timerEvent(event);
    }
}

void ScrollText::openTimer() {
    if (_timerId == 0 && _timerState == TimerState::Open) {
        _timerId = startTimer(_updateTime);
    }
}

void ScrollText::closeTimer() {
    if (_timerId) {
        killTimer(_timerId);
        _timerId = 0;
    }
}