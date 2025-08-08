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

#include <QQuickImageProvider>
#include <QHash>

namespace HX {

class ImagePoll : public QQuickImageProvider {
    Q_OBJECT

    ImagePoll()
        : QQuickImageProvider{QQuickImageProvider::Image}
    {}

    ImagePoll& operator=(ImagePoll&&) noexcept = delete;
public:
    static ImagePoll* get() noexcept {
        static auto* s = new ImagePoll; // 由 qml 释放
        return s;
    }

    QImage requestImage(
        [[maybe_unused]] QString const& id,
        [[maybe_unused]] QSize* size,
        [[maybe_unused]] QSize const& requestedSize
    ) override {
        auto it = _imgPool.find(id);
        if (it == _imgPool.end()) [[unlikely]] {
            return {}; // @todo 需要一个默认的加载失败的img
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
    QHash<QString, QImage> _imgPool;
};

} // namespace HX

#endif // !_HX_IMAGE_POOL_H_