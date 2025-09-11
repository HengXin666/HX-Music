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
#include <QString>

#include <HXLibs/log/Log.hpp>

namespace HX {

class MessageController : public QObject {
    Q_OBJECT

    explicit MessageController(QObject* parent = nullptr)
        : QObject{parent}
    {}
public:
    // 单例模式访问
    static MessageController& get() {
        static MessageController s{};
        return s;
    }

Q_SIGNALS:
    // 发送消息的信号
    void showMessageRequested(const QString& type, const QString& message);

public Q_SLOTS:
    // 显示信息消息
    void showInfo(const QString& message) {
        log::hxLog.info("infoooo");
        Q_EMIT showMessageRequested("info", message);
    }

    // 显示错误消息
    void showError(const QString& message) {
        Q_EMIT showMessageRequested("error", message);
    }

    // 显示警告消息
    void showWarning(const QString& message) {
        Q_EMIT showMessageRequested("warning", message);
    }

    // 显示成功消息
    void showSuccess(const QString& message) {
        Q_EMIT showMessageRequested("success", message);
    }
};

} // namespace HX