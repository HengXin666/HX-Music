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

#include <string>
#include <optional>

namespace HX::vo {

enum class VOCode : int {
    Err = -1,
    OK = 0
};

inline bool operator==(int v, VOCode code) noexcept {
    return v == static_cast<int>(code);
}

inline bool operator!=(int v, VOCode code) noexcept {
    return v != static_cast<int>(code);
}


template <typename T>
struct JsonVO {
    int code;
    std::string msg;
    std::optional<T> data;

    constexpr static JsonVO error(std::string msg) {
        return {
            static_cast<int>(VOCode::Err),
            std::move(msg),
            {}
        };
    }

    constexpr static JsonVO succeed(T&& t) {
        return {
            static_cast<int>(VOCode::OK),
            "ok",
            std::move(t)
        };
    }

    constexpr bool isError() const noexcept {
        return code != VOCode::OK;
    }
};

} // namespace HX::vo