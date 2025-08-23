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
#include <string_view>
#include <stdexcept>
#include <vector>

#include <sqlite3.h>

#include <HXLibs/reflection/MemberName.hpp>
#include <HXLibs/log/serialize/ToString.hpp>

#include <db/SQLiteStmt.hpp>

namespace HX::db {

namespace internal {

template <typename T>
constexpr std::string_view getSqlTypeStr() {
    if constexpr (std::is_integral_v<T>) {
        return "INTEGER";
    } else if constexpr (std::is_floating_point_v<T>) {
        return "REAL";
    } else if constexpr (meta::StringType<T>) {
        return "TEXT";
    } else {
        // 不支持该类型
        static_assert(!sizeof(T), "type is not sql type");
    }
}

} // namespace internal

class SQLiteDB {
public:
    explicit SQLiteDB(std::string_view filePath) {
        if (::sqlite3_open(filePath.data(), &_db) != SQLITE_OK) [[unlikely]] {
            throw std::runtime_error{
                "Failed to open database: " + std::string(sqlite3_errmsg(_db))
            };
        }
    }

    ~SQLiteDB() noexcept {
        if (_db) {
            ::sqlite3_close(_db);
        }
    }

    void exec(std::string_view sql) const {
        char* errMsg = nullptr;
        if (::sqlite3_exec(_db, sql.data(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string err = errMsg ? errMsg : "unknown error";
            ::sqlite3_free(errMsg);
            throw std::runtime_error{err};
        }
    }

    template <typename T>
    void createDatabase(std::string_view tableName) const {
        std::string sql = "CREATE TABLE IF NOT EXISTS ";
        sql += tableName;
        sql += " (";
        auto obj = reflection::internal::getStaticObj<T>();
        reflection::forEach(obj, [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto&& val
        ) {
            if constexpr (Idx > 0) {
                sql += ", ";
            }
            sql += internal::getSqlTypeStr<meta::remove_cvref_t<decltype(val)>>();
        });
        sql += ");";
        exec(sql);
    }

    template <typename T>
    void insert(std::string_view tableName, T&& t) const {
        std::string sql = "INSERT INTO ";
        sql += tableName;
        sql += " (";
        auto obj = reflection::internal::getStaticObj<T>();
        reflection::forEach(obj, [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto&& val
        ) {
            if constexpr (Idx > 0) {
                sql += ", ";
            }
            sql += internal::getSqlTypeStr<meta::remove_cvref_t<decltype(val)>>();
        });
        sql += ") VALUES (";
        reflection::forEach(std::forward<T>(t), [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto& val
        ) {
            using ValType = meta::remove_cvref_t<decltype(val)>;
            if constexpr (Idx > 0) {
                sql += ", ";
            }
            if constexpr (std::is_arithmetic_v<ValType>) {
                log::toString(val, sql);
            } else if constexpr (meta::StringType<ValType>) {
                sql += '\'';
                log::toString(static_cast<const char*>(val.data()), sql);
                sql += '\'';
            } else {
                // 不支持该类型
                static_assert(!sizeof(T), "type is not sql type");
            }
        });
        sql += ");";
        exec(sql);
    }

    template <typename T>
    std::vector<T> queryAll(std::string_view sql) const {
        SQLiteStmt stmt{sql, _db};
        std::vector<T> res;
        constexpr std::size_t N = reflection::membersCountVal<T>;
        for (int rc = stmt.step(); rc == SQLITE_ROW; rc = stmt.step()) {
            reflection::forEach(res.emplace_back(), [&] <std::size_t Idx> (
                std::index_sequence<Idx>, std::string_view, auto& val
            ) {
                using ValType = meta::remove_cvref_t<decltype(val)>;
                val = stmt.getColumnByIndex<ValType>(Idx);
            });
        }
        return res;
    }

private:
    ::sqlite3* _db{};
};

[[nodiscard]] inline SQLiteDB open(std::string_view filePath) {
    return SQLiteDB{filePath};
}

} // namespace HX::db