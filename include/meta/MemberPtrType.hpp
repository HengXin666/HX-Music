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

namespace HX::meta {

namespace internal {

template <typename T>
struct GetMemberPtrTypeImpl;

template <typename T, typename ClassPtr>
struct GetMemberPtrTypeImpl<T ClassPtr::*> {
    using Type = T;
};

} // namespace internal

/**
 * @brief 判断类型是否为成员指针类型
 * @tparam T 
 */
template <typename T>
bool constexpr IsMemberPtrVal = false;

template <typename T, typename ClassPtr>
bool constexpr IsMemberPtrVal<T ClassPtr::*> = true;

/**
 * @brief 获取成员指针指向的元素的类型
 * @tparam T 
 */
template <typename T>
using GetMemberPtrType = typename internal::GetMemberPtrTypeImpl<T>::Type;

} // namespace HX::meta