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

#include <array>
#include <cstdint>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace HX::utils {

struct Uuid {
    static std::string makeV4() noexcept {
        std::array<uint8_t, 16> bytes{};
        static thread_local std::mt19937_64 rng{std::random_device{}()};
        std::uniform_int_distribution<uint32_t> dist(0, 255);

        for (auto& b : bytes) {
            b = static_cast<uint8_t>(dist(rng));
        }

        // 设置版本号 (0100)
        bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
        // 设置变体 (10xx)
        bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < bytes.size(); ++i) {
            oss << std::setw(2) << static_cast<int>(bytes[i]);
            if (i == 3 || i == 5 || i == 7 || i == 9) {
                oss << "-";
            }
        }
        return std::move(oss).str();
    } 
};

} // namespace HX::utils