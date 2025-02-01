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
#ifndef _HX_MAIN_DISPLAY_BAR_H_
#define _HX_MAIN_DISPLAY_BAR_H_

#include <QStackedWidget>
#include <QPushButton>
#include <QStack>
#include <QPointer>

/**
 * @brief 主显示栏
 */
class MainDisplayBar : public QWidget {
    Q_OBJECT
public:
    explicit MainDisplayBar(QWidget* parent = nullptr);

private:
    // 页面栈, 当前控件永远只显示栈顶的控件
    QStack<QPointer<QWidget>> _pageStack {};
    QStackedWidget* _stackedWidget = new QStackedWidget(this);
    QPushButton* _btnPop = new QPushButton(this);
};

#endif // !_HX_MAIN_DISPLAY_BAR_H_