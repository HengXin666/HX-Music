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
#ifndef _HX_TOP_BAR_H_
#define _HX_TOP_BAR_H_

#include <QWidget>

#include <QLineEdit>
#include <QPushButton>

/**
 * @brief 顶部栏
 */
class TopBar : public QWidget {
    Q_OBJECT
public:
    explicit TopBar(QWidget* parent = nullptr);
private:
    // 搜索栏
    QPushButton* _btnSearch = new QPushButton(this);
    QLineEdit* _textSearch = new QLineEdit(this);
};

#endif // !_HX_TOP_BAR_H_