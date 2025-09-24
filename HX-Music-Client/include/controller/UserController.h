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
#include <api/UserApi.hpp>
#include <controller/MessageController.h>

namespace HX {

class UserController : public QObject {
    Q_OBJECT
public:
    UserController(QObject* p = nullptr)
        : QObject{p}
    {
        NetSingleton::get().setBackendUrl(
            GlobalSingleton::get().musicConfig.backendUrl
        );
        NetSingleton::get().setToken(
            GlobalSingleton::get().musicConfig.token
        );
        testToken();
    }

    void testToken() {
        UserApi::testTokenReq().thenTry([this](auto t) {
            QMetaObject::invokeMethod(
                QCoreApplication::instance(),
                [this, _t = std::move(t)] {
                    if (!_t) {
                        MessageController::get().show<MsgType::Warning>("凭证失效, 请重新登录");
                        logoutReq();
                        Q_EMIT SignalBusSingleton::get().gotoLoginViewSignal();
                    } else {
                        MessageController::get().show<MsgType::Info>("已登录");
                        _isLogin = true;
                        Q_EMIT loginChanged();
                    }
                },
                Qt::QueuedConnection
            );
        });
    }

    Q_INVOKABLE void loginReq(QString const& name, QString const& passwd) noexcept {
        UserApi::loginReq(name.toStdString(), passwd.toStdString())
            .thenTry([this](auto t) {
                QMetaObject::invokeMethod(
                    QCoreApplication::instance(),
                    [this, _t = std::move(t)] {
                        if (!_t) {
                            MessageController::get().show<MsgType::Error>(_t.what());
                            Q_EMIT SignalBusSingleton::get().gotoLoginViewSignal();
                        } else {
                            MessageController::get().show<MsgType::Success>("登录成功");
                            _isLogin = true;
                            Q_EMIT loginChanged();
                        }
                    },
                    Qt::QueuedConnection
                );
            });
    }

    Q_INVOKABLE void logoutReq() {
        NetSingleton::get().setToken(
            GlobalSingleton::get().musicConfig.token = ""
        );
        _isLogin = false;
        Q_EMIT loginChanged();
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

    Q_INVOKABLE bool isLoggedIn() const noexcept {
        return _isLogin;
    }

    // 修改用户名请求
    Q_INVOKABLE void updateNameReq(QString name) {
        UserApi::updateUserName(name.toStdString())
            .thenTry([this, _name = std::move(name)](auto t) {
                if (!t) [[unlikely]] {
                    MessageController::get().show<MsgType::Error>("修改用户名失败: " + t.what());
                    return;
                }
                setName(_name);
                MessageController::get().show<MsgType::Success>("修改用户名成功");
                Q_EMIT loginChanged();
            });
    }

    // 修改密码请求
    Q_INVOKABLE void updatePassword(QString oldPwd, QString newPwd) {
        UserApi::updatePasswd(
            oldPwd.toStdString(), newPwd.toStdString()
        ).thenTry([this](auto t) {
            if (!t) [[unlikely]] {
                MessageController::get().show<MsgType::Error>("修改密码失败: " + t.what());
                return;
            }
            MessageController::get().show<MsgType::Success>("修改密码成功");
            testToken();
        });
    }

Q_SIGNALS:
    void backendUrlChanged();
    void nameChanged();
    void loginChanged();

private:
    bool _isLogin = false;
};

} // namespace HX