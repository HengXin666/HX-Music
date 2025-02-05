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

/**
 * @brief 支持svg变色的按钮
 */
class SvgIconPushButton : public QPushButton {
    Q_OBJECT
public:
    /**
     * @brief 支持svg变色的按钮
     * @param svgPath svg文件的路径
     * @param ordinary 平常的颜色
     * @param hover 按下的颜色
     * @param parent 父控件
     */
    explicit SvgIconPushButton(
        const QString& svgPath,
        const QColor& ordinary,
        const QColor& hover,
        QWidget* parent = nullptr
    );

private:
    QString _svgPath;
    QIcon _ordinaryIcon;
    QIcon _hoverIcon;
};

#endif // !_HX_SVG_ICON_PUSH_BUTTON_H_