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
#ifndef _HX_TUPLE_ELEMENT_EXTRACTOR_H_
#define _HX_TUPLE_ELEMENT_EXTRACTOR_H_

#include <tuple>
#include <vector>
#include <type_traits>

namespace HX {

/**
 * @brief Tuple元素提取类
 */
struct TupleElementExtractor {
    /**
     * @brief 提取`vector<tuple<Ts...>>`为`vector<get<Index>(tuple<Ts...>)>`
     * @tparam Ts 
     * @tparam Index 需要提取的`tuple`索引
     * @param arr 
     * @return auto 
     */
    template <std::size_t Index, typename... Ts>
    inline static auto extractorToVector(
        std::vector<std::tuple<Ts...>>& arr
    ) {
        using T = std::decay_t<decltype(std::get<Index>(std::declval<std::tuple<Ts...>>()))>;
        std::vector<T> res;
        res.reserve(arr.size());
        for (auto&& it : arr)
            res.emplace_back(std::move(std::get<Index>(it)));
        return res;
    }
};

} // namespace HX

#endif // !_HX_TUPLE_ELEMENT_EXTRACTOR_H_