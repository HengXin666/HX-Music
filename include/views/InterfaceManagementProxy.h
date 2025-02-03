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
#ifndef _HX_INTERFACE_MANAGEMENT_PROXY_H_
#define _HX_INTERFACE_MANAGEMENT_PROXY_H_

#include <QWidget>

#include <widget/MainDisplayBar.h>

/**
 * @brief 界面管理代理类
 */
class InterfaceManagementProxy {
public:
    void setMainDisplayBar(MainDisplayBar* m) {
        _mainDisplayBar = m;
    }

    /**
     * @brief 添加页面并且显示
     * @param view 页面
     */
    void pushView(QWidget* view) {
        _mainDisplayBar->_stackedWidget->addWidget(view);
        _mainDisplayBar->_stackedWidget->setCurrentWidget(view);
    }

    /**
     * @brief 删除当前显示的页面, 然后显示之后的栈顶界面
     */
    void popView() {
        if (_mainDisplayBar->_stackedWidget->currentIndex() > 0) {
            auto* delView = _mainDisplayBar->_stackedWidget->currentWidget();
            _mainDisplayBar->_stackedWidget->removeWidget(
                delView
            );
            delete delView;
        }
    }
private:
    MainDisplayBar* _mainDisplayBar = nullptr;
};

#endif // !_HX_INTERFACE_MANAGEMENT_PROXY_H_