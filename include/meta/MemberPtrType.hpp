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

#include <type_traits>

namespace HX::meta {

namespace internal {

template <typename T>
struct GetMemberPtrTypeImpl;

template <typename T, typename ClassT>
struct GetMemberPtrTypeImpl<T ClassT::*> {
    using Type = T;
    using ClassType = ClassT;
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

/**
 * @brief 获取成员指针的类的类型
 * @tparam T 
 */
template <typename T>
using GetMemberPtrClassType = typename internal::GetMemberPtrTypeImpl<T>::ClassType;

/**
 * @brief 获取成员指针们的类的类型, 并且保证这些指针都来自于同一个类
 * @tparam MemberPtr 
 * @tparam MemberPtrs
 */
template <typename... MemberPtrTs>
using GetMemberPtrsClassType = decltype([] <typename MP, typename... MemberPtrs> (MP, MemberPtrs...) {
    // 别名模板不支持部分推导
    // 不能 <typename MP, typename... MemberPtrs> 然后直接让 <typename... MemberPtrTs> 匹配
    // 只能拆开
    using MemberPtr = MP;
    return std::conditional_t<
    (std::is_same_v<
        GetMemberPtrClassType<MemberPtr>,
        GetMemberPtrClassType<MemberPtrs>
    > && ...),
    GetMemberPtrClassType<MemberPtr>, void> {};
} (MemberPtrTs{}...));

} // namespace HX::meta