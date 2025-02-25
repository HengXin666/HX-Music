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
#ifndef _HX_SCROLL_TEXT_H_
#define _HX_SCROLL_TEXT_H_

#include <cstdint>

#include <QLabel>

/**
 * @brief 滚动的文本 (如果文本长度大于控件长度, 则会滚动), 可停止
 */
class ScrollText : public QLabel {
    Q_OBJECT
public:
    explicit ScrollText(QWidget* parent = nullptr);

    void setText(const QString& text);

    QSize sizeHint() const override {
        return fontMetrics().size(0, _text);  // 确保返回正确大小
    }

    /**
     * @brief 设置滚动间隙
     * @param gap 
     */
    void setGap(unsigned int gap) {
        _gap = gap;
    }

    /**
     * @brief 设置滚动结束, 暂停显示的时间
     * @param time (单位: 毫秒(ms))
     */
    void setPauseTime(unsigned int time) {
        _pauseTime = time;
    }

    /**
     * @brief 设置`刷新间隔/滚动间隔`的时间
     * @param time (单位: 毫秒(ms))
     */
    void setUpdateTime(unsigned int time) {
        _updateTime = time;
    }

protected:
    /**
     * @brief 鼠标离开事件
     * @param event 
     */
    void leaveEvent(QEvent* event) override {
        _isMouseInside = false;
        openTimer();
        QLabel::leaveEvent(event);
    }

    /**
     * @brief 鼠标进入事件
     * @param event 
     */
    void enterEvent(QEnterEvent* event) override {
        _isMouseInside = true;
        closeTimer();
        QLabel::enterEvent(event);
    }

    void paintEvent(QPaintEvent* event) override;

    void timerEvent(QTimerEvent* event) override;

    void showEvent(QShowEvent *event) override {
        Q_UNUSED(event);
        openTimer();
    }

    void hideEvent(QHideEvent *event) override {
        Q_UNUSED(event);
        closeTimer();
    }

private:
    /**
     * @brief 开启滚动定时器
     */
    void openTimer();

    /**
     * @brief 关闭滚动定时器
     */
    void closeTimer();

    unsigned int _gap = 100;            // 间隙
    unsigned int _pauseTime = 0;        // 滚动结束, 停止显示的时间 (单位: 毫秒(ms))
    unsigned int _updateTime = 20;      // 滚动间隔 (单位: 毫秒(ms))
    enum class TimerState : u_int16_t {
        Open,
        Close,
    } _timerState = TimerState::Close;  // 定时器状态
    bool _isMouseInside = false;        // 鼠标是否在控件内部
    int _offset = 0;                    // 偏移量
    int _timerId = 0;                   // 定时器id
    QString _text;                      // 存储文本内容
};


#endif // !_HX_SCROLL_TEXT_H_