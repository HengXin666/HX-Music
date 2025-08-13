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
#ifndef _HX_WINDOW_MASK_UTIL_H_
#define _HX_WINDOW_MASK_UTIL_H_

#include <QObject>
#include <QWindow>
#include <QPolygon>
#include <QRegion>

#include <QDebug>

namespace HX {

class WindowMaskUtil : public QObject {
    Q_OBJECT
public:
    explicit WindowMaskUtil(QObject* parent = nullptr)
        : QObject(parent)
    {}

    Q_INVOKABLE void clear(QWindow* window) {
        _regions = {};
        if (window) {
            window->setMask(_regions);
        }
    }

    Q_INVOKABLE void addControlRect(int x, int y, int width, int height) {
        _regions = QRegion(
            QRect(x, y, width, height)
        );
        qDebug() << x << y << width << height;
    }

    Q_INVOKABLE void setMask(QWindow* window) {
        qDebug() << "设了吗";
        if (!window || _regions.isEmpty()) {
            qDebug() << (window ? "win sb" : "reg sb");
            return;
        }
        qDebug() << "设了";
        window->setMask(_regions);
    }
private:
    QRegion _regions;
};

} // namespace HX

#endif // !_HX_WINDOW_MASK_UTIL_H_