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
#ifndef _HX_IMAGE_POOL_H_
#define _HX_IMAGE_POOL_H_

#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQuickImageProvider>
#include <QHash>

#include <api/CoverApi.hpp>
#include <controller/MessageController.h>

namespace HX {

/**
 * @brief 网络图片池 [封面]
 *  - 纯数字就是歌曲封面
 */
class OnlineImagePoll : public QQuickImageProvider {
    Q_OBJECT

    OnlineImagePoll()
        : QQuickImageProvider{QQuickImageProvider::Image}
        , _noFindImg{":/icons/audio.svg"}
        , _imgPool{}
    {}

    OnlineImagePoll& operator=(OnlineImagePoll&&) noexcept = delete;
public:
    static OnlineImagePoll* get() noexcept {
        static auto* s = new OnlineImagePoll; // 由 qml 释放
        return s;
    }

    QImage requestImage(
        [[maybe_unused]] QString const& id,
        [[maybe_unused]] QSize* size,
        [[maybe_unused]] QSize const& requestedSize
    ) override {
        auto it = _imgPool.find(id);
        if (it == _imgPool.end()) {
            bool ok = false;
            auto uint64Id = id.toULongLong(&ok);
            if (ok) [[likely]] {
                // 请求封面
                return CoverApi::getCoverImg(uint64Id)
                    .thenTry([this, _idStr = id](container::Try<QImage> t) {
                        if (!t) [[unlikely]] {
                            MessageController::get().show<MsgType::Error>("封面失败:" + t.what());
                            return _noFindImg;
                        }
                        auto res = t.get();
                        QMetaObject::invokeMethod(
                            QCoreApplication::instance(),
                            [this, idStr = std::move(_idStr), img = t.move()]() mutable {
                            add(idStr, std::move(img));
                        });
                        return res;
                    }).get();
            }
            return _noFindImg;
        }
        auto& res = it.value();
        if (size) {
            *size = res.size();
        }
        return res;
    }

    void add(QString const& key, QImage&& img) {
        _imgPool.emplace(key, std::move(img));
    }

    void erase(QString const& key) {
        _imgPool.remove(key);
    }
private:
    QImage _noFindImg;
    QHash<QString, QImage> _imgPool;
};

} // namespace HX

#endif // !_HX_IMAGE_POOL_H_