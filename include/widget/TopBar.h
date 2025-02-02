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
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QEvent>

/**
 * @brief 顶部栏
 */
class TopBar : public QWidget {
    Q_OBJECT
public:
    explicit TopBar(QWidget* parent = nullptr);

protected:
    /**
     * @brief 监听根窗口事件, 如果改变了窗口状态, 就看看是否需要修改图标
     * @param obj 
     * @param event 
     * @return true 
     * @return false 
     */
    bool eventFilter(QObject *obj, QEvent *event) override {
        QWidget *mainWindow = this->window();
        if (obj == mainWindow && event->type() == QEvent::WindowStateChange) {
            updateMaximizeIcon();
        }
        return QWidget::eventFilter(obj, event);
    }

private:
    /**
     * @brief 窗口最大化
     */
    void toggleMaximize() {
        QWidget *mainWindow = this->window();
        if (!mainWindow) 
            return;
        if (mainWindow->isMaximized()) {
            mainWindow->showNormal();
        } else {
            mainWindow->showMaximized();
        }
        updateMaximizeIcon();
    }

    /**
     * @brief 还原窗口
     */
    void updateMaximizeIcon() {
        QWidget *mainWindow = this->window();
        if (!mainWindow) 
            return;
        if (mainWindow->isMaximized()) {
            _btnMaximize->setIcon(QIcon(":/icons/restore.svg"));
        } else {
            _btnMaximize->setIcon(QIcon(":/icons/up.svg"));
        }
    }

    // 搜索栏
    QPushButton* _btnSearch = new QPushButton(this);
    QLineEdit* _textSearch = new QLineEdit(this);

    // 头像 用户名 等级
    QLabel* _imgAvatar = new QLabel(this);
    QLabel* _textUsername = new QLabel(this);
    QLabel* _textLevel = new QLabel(this);

    // 消息 更多(设置/...反正是个选项卡)
    QPushButton* _btnMsg = new QPushButton(this);
    QToolButton* _toolBtn = new QToolButton(this);

    // 隐藏 最大化 关闭
    QPushButton* _btnHide = new QPushButton(this);
    QPushButton* _btnMaximize = new QPushButton(this);
    QPushButton* _btnClose = new QPushButton(this);
};

#endif // !_HX_TOP_BAR_H_