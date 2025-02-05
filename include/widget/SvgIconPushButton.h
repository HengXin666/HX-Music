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
#ifndef _HX_SVG_ICON_PUSH_BUTTON_H_
#define _HX_SVG_ICON_PUSH_BUTTON_H_

#include <QPushButton>

#include <graphics/SvgIconEngine.hpp>

/**
 * @brief 支持svg变色的按钮
 */
class SvgIconPushButton : public QPushButton {
    Q_OBJECT
public:
    explicit SvgIconPushButton(
        const QString& svgPath, 
        const QSize& size, 
        QWidget* parent = nullptr
    )
        : QPushButton(parent)
        , _iconEngine(new SvgIconEngine(svgPath, size))
    {
        _iconEngine->switchColor("#990099");
        setIcon(QIcon(_iconEngine));
    }

    explicit SvgIconPushButton(const QString& svgPath, QWidget* parent = nullptr)
        : SvgIconPushButton(svgPath, {24, 24}, parent)
    {}

    void switchColor(const QColor& color) {
        _iconEngine->switchColor(color);
        setIcon(QIcon(_iconEngine));
    }

    void setSize(const QSize& size) {
        _iconEngine->setSize(size);
        setIcon(QIcon(_iconEngine));
    }

private:
    SvgIconEngine* _iconEngine;
};

#endif // !_HX_SVG_ICON_PUSH_BUTTON_H_