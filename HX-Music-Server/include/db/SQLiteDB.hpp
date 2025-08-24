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
#include <HXLibs/reflection/TypeName.hpp>
#include <HXLibs/log/serialize/ToString.hpp>

#include <db/SQLiteStmt.hpp>

namespace HX::db {

namespace internal {

enum class SqlOp : uint8_t {
    Where   = 1 << 0,
    GroupBy = 1 << 1,
    Having  = 1 << 2,
    OrderBy = 1 << 3,
    Limit   = 1 << 4,
};


inline constexpr uint8_t operator&(uint8_t a, SqlOp b) noexcept {
    return a & static_cast<uint8_t>(b);
}

inline constexpr uint8_t& operator|=(uint8_t& a, SqlOp b) noexcept {
    return a |= static_cast<uint8_t>(b);
}

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

inline void execSql(std::string_view sql, ::sqlite3* db) {
    char* errMsg = nullptr;
    if (::sqlite3_exec(db, sql.data(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "unknown error";
        ::sqlite3_free(errMsg);
        throw std::runtime_error{err};
    }
}

/**
 * @brief 解析 SQL 的占位符对应的字段名称
 * @param sql 
 * @return std::vector<std::string_view> 
 */
inline constexpr std::vector<std::string_view> extractFields(std::string_view sql) {
    std::vector<std::string_view> fields;
    for (size_t pos = 0; pos != std::string_view::npos; pos = sql.find('?', ++pos)) {
        auto it = sql.begin() + pos;

        // 向前找非字母数字
        auto rIt = std::find_if_not(
            std::make_reverse_iterator(it), sql.rend(),
            [](unsigned char c) {
                return std::isspace(c) || std::ispunct(c);
            });

        // 再向前找空格或特殊字符(即找到字段名的起点)
        auto lIt = std::find_if(
            rIt, sql.rend(), [](unsigned char c) {
                return !std::isalnum(c) && c != '_';
            });

        if (rIt != sql.rend()) {
            std::string_view field{lIt.base(), rIt.base()};
            if (!field.empty()) {
                fields.push_back(field);
            }
        }
    }
    return fields;
}

template <typename T>
class SqlCallChain {
    
public:
    SqlCallChain(std::string sql)
        : _sql{std::move(sql)}
    {}

    SqlCallChain& where(std::string_view sql) {
        if (_opEd & SqlOp::Where) [[unlikely]] {
            throw std::runtime_error{"@todo"};
        }
        _opEd |= SqlOp::Where;
        auto fields = extractFields(sql);
        
        return *this;
    }
private:
    std::string _sql;
    uint8_t _opEd;
};

} // namespace internal

class SQLiteDB {
    void exec(std::string_view sql) const {
        internal::execSql(sql, _db);
    }
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

    template <typename T>
    void createDatabase() const {
        std::string sql = "CREATE TABLE IF NOT EXISTS ";
        sql += reflection::getTypeName<T>();
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
    void insert(T&& t) const {
        using U = meta::remove_cvref_t<T>;
        std::string sql = "INSERT INTO ";
        sql += reflection::getTypeName<U>();
        sql += " (";
        auto obj = reflection::internal::getStaticObj<U>();
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
    std::vector<T> queryAll() const {
        std::string sql = "SELECT * FROM ";
        sql += reflection::getTypeName<T>();
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

    template <typename T>
    auto deleteBy() {
        std::string sql = "DELETE FROM ";
        sql += reflection::getTypeName<T>();
        return internal::SqlCallChain<T>{std::move(sql)};
    }

private:
    ::sqlite3* _db{};
};

[[nodiscard]] inline SQLiteDB open(std::string_view filePath) {
    return SQLiteDB{filePath};
}

} // namespace HX::db