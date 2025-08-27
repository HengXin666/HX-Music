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
#include <string_view>

namespace HX::meta {
    
// 一个承载 char 参数包的类型
template <char... Cs>
struct CharPack {
    inline static constexpr std::array<char, sizeof...(Cs) + 1> value = {Cs..., '\0'};
    inline static constexpr std::size_t size = sizeof...(Cs);

    static constexpr auto view() noexcept {
        return std::string_view{value.data(), size};
    }
};

// 可作为非类型模板参数的固定字符串类型
template <std::size_t N>
struct FixedString {
    char data[N] {};

    // 接收字面量拷贝到 data
    constexpr FixedString(const char(&s)[N]) {
        for(std::size_t i = 0; i < N; ++i) 
            data[i] = s[i];
    }

    // 长度不含终止符
    static consteval std::size_t literalSize() noexcept { return N; }

    static consteval std::size_t size() noexcept { return N - 1; }

    // 编译期索引访问
    consteval char operator[](std::size_t i) const noexcept { return data[i]; }

    // 为了结构化类型比较, 避免某些实现细节陷阱
    constexpr auto operator<=>(const FixedString&) const noexcept = default;
};

namespace internal {

// 把 FixedString<S> 编译期展开为 CharPack<S[0], S[1], ...>
template <FixedString S, std::size_t... I>
consteval auto toCharPackImpl(std::index_sequence<I...>) -> CharPack<S[I]...>;

} // namespace internal

template <FixedString S>
using ToCharPack = decltype(internal::toCharPackImpl<S>(std::make_index_sequence<S.size()>{}));

} // namespace HX::meta