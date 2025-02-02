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
#ifndef _HX_DIVIDER_WIDGET_H_
#define _HX_DIVIDER_WIDGET_H_

#include <QWidget>
#include <QPainter>

/**
 * @brief 分割线
 */
class DividerWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit DividerWidget(QWidget* parent = nullptr) 
        : QWidget(parent) 
    {}

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        // 设置画笔颜色和样式
        painter.setPen(QPen(Qt::gray, 2));
        // 绘制一条横跨整个控件宽度的直线
        painter.drawLine(0, height() / 2, width(), height() / 2);
    }
};

#endif // !_HX_DIVIDER_WIDGET_H_