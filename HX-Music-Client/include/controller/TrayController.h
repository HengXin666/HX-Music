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

#include <QObject>
#include <QSystemTrayIcon>
#include <QQmlApplicationEngine>
#include <QMenu>
#include <QApplication>

#include <HXLibs/log/Log.hpp>

namespace HX {

class TrayController : public QObject {
    Q_OBJECT
public:
    explicit TrayController(QObject* parent = nullptr)
        : QObject{parent}
        , trayIcon{new QSystemTrayIcon{this}}
        , trayMenu{new QMenu{}}
    {
        trayIcon->setIcon(QIcon(":/logo/trayIcon.png"));
        trayIcon->setToolTip("HX.Music");
        trayIcon->setContextMenu(trayMenu);
        trayIcon->show();
    }

    void init(QQmlApplicationEngine* engine) {
        // 获取 QML 根对象
        QObject* rootObj = engine->rootObjects().first();
        auto addToRootQml = [&](QString const& name, std::string_view funcSv) {
            QAction* action = trayMenu->addAction(name);
            connect(action, &QAction::triggered, [rootObj, funcSv]() {
                QMetaObject::invokeMethod(rootObj, funcSv.data());
            });
        };

        addToRootQml("显示", "onTrayShow");
        trayMenu->addSeparator();
        addToRootQml("播放/暂停", "onTrayTogglePause");
        addToRootQml("上一首", "onTrayPrev");
        addToRootQml("下一首", "onTrayNext");
        trayMenu->addSeparator();
        addToRootQml("退出", "onTrayClose");
    }

    Q_INVOKABLE void showMessage(const QString& title, const QString& message) {
        trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    }

private:
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
};

} // namespace HX