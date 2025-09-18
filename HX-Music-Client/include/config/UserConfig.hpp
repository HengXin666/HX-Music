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
#include <QApplication>

#include <singleton/GlobalSingleton.hpp>
#include <singleton/NetSingleton.hpp>
#include <singleton/SignalBusSingleton.h>
#include <controller/MessageController.h>
#include <api/UserApi.hpp>

namespace HX {

class UserConfig : public QObject {
    Q_OBJECT
public:
    UserConfig(QObject* p = nullptr)
        : QObject{p}
    {
        NetSingleton::get().setBackendUrl(
            GlobalSingleton::get().musicConfig.backendUrl
        );
        NetSingleton::get().setToken(
            GlobalSingleton::get().musicConfig.token
        );
        UserApi::testTokenReq().thenTry([](auto t) {
            QMetaObject::invokeMethod(
                QCoreApplication::instance(),
                [_t = std::move(t)] {
                    if (!_t) {
                        MessageController::get().show<MsgType::Warning>("凭证失效, 请重新登录");
                        Q_EMIT SignalBusSingleton::get().gotoLoginViewSignal();
                    } else {
                        MessageController::get().show<MsgType::Info>("已登录");
                    }
                },
                Qt::QueuedConnection
            );
        });
    }

    Q_INVOKABLE QString getBackendUrl() const noexcept {
        return QString::fromStdString(NetSingleton::get().getBackendUrl());
    }

    Q_INVOKABLE void setBackendUrl(QString const& url) {
        NetSingleton::get().setBackendUrl(
            GlobalSingleton::get().musicConfig.backendUrl = url.toStdString()
        );
        Q_EMIT backendUrlChanged();
    }

    Q_INVOKABLE QString getName() const noexcept {
        return QString::fromStdString(GlobalSingleton::get().musicConfig.name);
    }

    Q_INVOKABLE void setName(QString const& name) {
        GlobalSingleton::get().musicConfig.name = name.toStdString();
        Q_EMIT nameChanged();
    }

Q_SIGNALS:
    void backendUrlChanged();
    void nameChanged();
};

} // namespace HX