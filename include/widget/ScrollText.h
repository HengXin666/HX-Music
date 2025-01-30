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

#include <QLabel>

/**
 * @brief 滚动文本 (如果文本长度大于控件长度, 则会滚动)
 */
class ScrollText : public QLabel {
    Q_OBJECT
public:
    explicit ScrollText(QWidget* parent = nullptr);
};

#endif // !_HX_SCROLL_TEXT_H_