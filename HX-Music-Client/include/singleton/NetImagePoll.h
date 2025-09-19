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
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQuickImageProvider>

#include <api/Api.hpp>
#include <singleton/NetSingleton.hpp>
#include <controller/MessageController.h>

namespace HX {

/**
 * @brief 加载网络图片, 无缓存. 请在qml侧开启缓存~
 */
class NetImagePoll : public QQuickImageProvider {
    Q_OBJECT
public:
    NetImagePoll()
        : QQuickImageProvider{QQuickImageProvider::Image}
        , _errImg{":/icons/img_err.svg"}
    {}

    NetImagePoll& operator=(NetImagePoll&&) noexcept = delete;

    QImage requestImage(
        [[maybe_unused]] QString const& url,
        [[maybe_unused]] QSize* size,
        [[maybe_unused]] QSize const& requestedSize
    ) override {
        auto stdUrl = url.split('?')[0].toStdString();
        return NetSingleton::get().getReq("/" + stdUrl)
            .thenTry([&](container::Try<net::ResponseData> t) -> QImage {
                if (!t) [[unlikely]] {
                    MessageController::get().show<MsgType::Error>("加载图片 [" + stdUrl + "] 失败: 网络错误");
                    return _errImg;
                } else if (t.get().status / 100 != 2) [[unlikely]] {
                    MessageController::get().show<MsgType::Error>("加载图片 [" + stdUrl + "] 失败: status != 200");
                    return _errImg;
                }
                QImage res{};
                auto imgBuf = std::move(t.move().body); 
                if (res.loadFromData(QByteArrayView{
                    imgBuf.data(), static_cast<qint64>(imgBuf.size())})
                ) {
                    return res;
                } else {
                    MessageController::get().show<MsgType::Error>("加载图片 [" + stdUrl + "] 失败: 图片格式错误");
                    return _errImg;
                }
            }).get();
    }
private:
    QImage _errImg;
};

} // namespace HX
