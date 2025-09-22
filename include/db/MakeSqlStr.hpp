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

#include <meta/MemberPtrType.hpp>
#include <db/SQLiteMeta.hpp>
#include <HXLibs/meta/TypeTraits.hpp>
#include <HXLibs/reflection/MemberPtr.hpp>
#include <HXLibs/reflection/TypeName.hpp>

namespace HX::db {

struct MakeSqlStr {
    template <typename T, bool IsSetPrimaryKey = false>
    static std::string makeInsertSql() noexcept {
        using U = meta::remove_cvref_t<T>;
        std::string sqlName = "INSERT INTO ";
        sqlName += reflection::getTypeName<U>();
        sqlName += " (";
        constexpr auto& obj = reflection::internal::getStaticObj<U>();
        constexpr auto N = reflection::membersCountVal<U>;
        std::string sqlVal = ") VALUES (";
        reflection::forEach(obj, [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto&& val
        ) constexpr {
            using Type = meta::remove_cvref_t<decltype(val)>;
            if constexpr (isPrimaryKeyVal<Type> && !IsSetPrimaryKey) {
                // 主键不会进行指定
                return;
            } else {
                sqlName += name;
                sqlVal += '?';
                if constexpr (Idx + 1 != N) {
                    sqlVal += ',';
                    sqlName += ',';
                }
            }
        });
        sqlName += std::move(sqlVal);
        sqlName += ");";
        return sqlName;
    }

    template <typename T, typename... MemberPtr>
        requires (sizeof...(MemberPtr) >= 1 && (meta::IsMemberPtrVal<MemberPtr> && ...))
    static std::string makeInsertSql(MemberPtr... ptrs) noexcept {
        using U = meta::remove_cvref_t<T>;
        std::string sqlName = "INSERT INTO ";
        sqlName += reflection::getTypeName<U>();
        sqlName += " (";
        constexpr auto& obj = reflection::internal::getStaticObj<U>();
        constexpr auto N = reflection::membersCountVal<U>;
        std::string sqlVal = ") VALUES (";
        auto map = reflection::makeMemberPtrToNameMap<U>();
        (([&](){
            sqlName += map.at(ptrs);
            sqlName += ',';
            sqlVal += '?';
            sqlVal += ',';
        }()), ...);
        sqlName.pop_back();
        sqlVal.back() = ')';
        sqlName += std::move(sqlVal);
        sqlName += ';';
        return sqlName;
    }

    template <typename T, bool IsSetPrimaryKey = false>
    static std::string makeUpdateSqlFragment() noexcept {
        using U = meta::remove_cvref_t<T>;
        std::string sql = "UPDATE ";
        sql += reflection::getTypeName<U>();
        sql += " SET ";
        constexpr auto& obj = reflection::internal::getStaticObj<U>();
        constexpr auto N = reflection::membersCountVal<U>;
        reflection::forEach(obj, [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto&& val
        ) constexpr {
            using Type = meta::remove_cvref_t<decltype(val)>;
            if constexpr (isPrimaryKeyVal<Type> && !IsSetPrimaryKey) {
                // 主键不会进行指定
                return;
            } else {
                sql += name;
                sql += '=';
                sql += '?';
                if constexpr (Idx + 1 != N) {
                    sql += ',';
                } else {
                    sql += ' ';
                }
            }
        });
        return sql;
    }

    template <typename T, typename... MemberPtr>
        requires (sizeof...(MemberPtr) >= 1 && (meta::IsMemberPtrVal<MemberPtr> && ...))
    static std::string makeUpdateSqlFragment(MemberPtr... ptrs) noexcept {
        using U = meta::remove_cvref_t<T>;
        std::string sql = "UPDATE ";
        sql += reflection::getTypeName<U>();
        sql += " SET ";
        auto map = reflection::makeMemberPtrToNameMap<U>();
        (([&](){
            sql += map.at(ptrs);
            sql += '=';
            sql += '?';
            sql += ',';
        }()), ...);
        sql.back() = ' ';
        return sql;
    }
};

} // namespace HX::db