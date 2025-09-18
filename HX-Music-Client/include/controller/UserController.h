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

#include <api/UserApi.hpp>
#include <controller/MessageController.h>

namespace HX {

class UserController : public QObject {
    Q_OBJECT
public:
    UserController(QObject* p = nullptr)
        : QObject{p}
    {}

    Q_INVOKABLE void loginReq(QString const& name, QString const& passwd) const noexcept {
        UserApi::loginReq(name.toStdString(), passwd.toStdString())
            .thenTry([](auto t) {
                if (!t) {
                    MessageController::get().show<MsgType::Error>(t.what());
                } else {
                    MessageController::get().show<MsgType::Success>("登录成功");
                }
            });
    }
};

} // namespace HX