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

#include <chrono>

namespace HX::utils {

struct Timestamp final {
    
    /**
     * @brief 获取当前时间戳
     * @tparam DurationType 
     * @return int64_t 
     */
    template <typename DurationType = std::chrono::milliseconds>
    constexpr static int64_t getTimestamp() {
        return std::chrono::duration_cast<DurationType>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    /**
     * @brief 时间戳转 STD 持续时间
     * @tparam DurationType 
     * @param timestamp 
     * @return DurationType 
     */
    template <typename DurationType = std::chrono::milliseconds>
    constexpr static DurationType timestampNumToStdChrono(int64_t timestamp) {
        return DurationType{static_cast<typename DurationType::rep>(timestamp)};
    }

    /**
     * @brief 时间戳转 STD 绝对时间点
     * @tparam DurationType 
     * @param timestamp 
     * @return std::chrono::time_point<
     * std::chrono::system_clock,
     * DurationType
     * > 
     */
    template <typename DurationType = std::chrono::milliseconds>
    constexpr static std::chrono::time_point<
        std::chrono::system_clock,
        DurationType
    > timestampNumToStdChrono(int64_t timestamp) {
        return std::chrono::time_point<std::chrono::system_clock, DurationType>{
            DurationType{static_cast<typename DurationType::rep>(timestamp)}
        };
    }
};

} // namespace HX::utils