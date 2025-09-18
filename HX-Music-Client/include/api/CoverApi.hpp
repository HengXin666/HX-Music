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

#include <QImage>

#include <singleton/NetSingleton.hpp>

#include <api/Api.hpp>

namespace HX {

/**
 * @brief 歌曲封面相关请求 API
 */
struct CoverApi {
    static container::FutureResult<QImage> getCoverImg(uint64_t id) {
        return NetSingleton::get().getReq("/cover/select/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) {
                if (!t) [[unlikely]] {
                    t.rethrow();
                } else if (t.get().status != 200) [[unlikely]] {
                    api::throwVoMsg(t.move());
                }
                QImage res{};
                auto imgBuf = std::move(t.move().body); 
                if (res.loadFromData(QByteArrayView{
                    imgBuf.data(), static_cast<qint64>(imgBuf.size())})) {
                        return res;
                } else {
                    throw std::runtime_error{"QT: Bad Img"};
                }
            });
    }
};

} // namespace HX