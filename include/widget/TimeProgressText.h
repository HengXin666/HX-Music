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
#ifndef _HX_TIME_PROGRESS_TEXT_H_
#define _HX_TIME_PROGRESS_TEXT_H_

#include <QWidget>
#include <QPainter>

/**
 * @brief 播放时间进度文本, 如: 播放时长/总时长
 */
class TimeProgressText : public QWidget {
    Q_OBJECT
public:
    explicit TimeProgressText(QWidget* parent = nullptr);

    /**
     * @brief 更新总时长
     * @param totalTime 
     */
    void updateTotalTime(QString const& totalTime) {
        _totalTime = totalTime;
        update();
    }

    /**
     * @brief 更新当前播放时长
     * @param nowTime
     */
    void updateNowTime(QString const& nowTime) {
        _nowTime = nowTime;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 黑体
        QFont font("Boldface", 8);
        painter.setFont(font);
        painter.setPen(Qt::white);
        
        // 显示当前播放时间 / 总时长
        painter.drawText(rect(), Qt::AlignCenter, _nowTime + " / " + _totalTime);
    }

private:
    QString _nowTime = "00:00";   // 播放时长
    QString _totalTime = "00:00"; // 总时长
};

#endif // !_HX_TIME_PROGRESS_TEXT_H_