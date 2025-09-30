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

#if 0

#include <atomic>

namespace HX::meta {

struct TypeId {
    // 适用于 类成员指针
    template <auto Vs>
    static std::size_t make() noexcept {
        static const auto id = get();
        return id;
    }

    template <typename... Ts>
    static std::size_t make() noexcept {
        static const auto id = get();
        return id;
    }
private:
    static std::size_t get() noexcept {
        static TypeId s{};
        return s._cnt.fetch_add(1, std::memory_order_relaxed);
    }
    TypeId() = default;
    TypeId& operator=(TypeId&&) = delete;
    std::atomic_size_t _cnt{0};
};

} // namespace HX::meta

#else

#include <HXLibs/reflection/MemberName.hpp>

namespace HX::meta {

namespace internal {

template <auto Vs>
struct StaticVal {
    inline static constexpr auto Val = Vs;
};

} // namespace internal

struct TypeId {
    using IdType = void const * const;

    template <auto Vs>
    static consteval IdType make() noexcept {
        return static_cast<IdType>(&internal::StaticVal<Vs>::Val);
    }

    template <typename T>
    static consteval IdType make() noexcept {
        return static_cast<IdType>(&reflection::internal::getStaticObj<T>());
    }
};

} // namespace HX::meta

#endif