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

#include <sqlite3.h>

#include <db/SQLiteMeta.hpp>
#include <HXLibs/meta/ContainerConcepts.hpp>

namespace HX::db {

class SQLiteStmt {
public:
    SQLiteStmt(std::string_view sql, ::sqlite3* db)
        : _stmt{nullptr}
    {
        // 预编译
        if (::sqlite3_prepare_v2(
            db, sql.data(), sql.size(),
            &_stmt, nullptr) != SQLITE_OK
        ) [[unlikely]] {
            throw std::runtime_error{
                "Failed to prepare statement: " 
                + std::string(::sqlite3_errmsg(db))
                + "\n In: " + std::string{sql}
            };
        }
    }

    SQLiteStmt(SQLiteStmt&) = delete;
    SQLiteStmt(SQLiteStmt&& that) noexcept
        : _stmt{that._stmt}
    {
        that._stmt = nullptr;
    }

    int step() const noexcept {
        return ::sqlite3_step(_stmt);
    }

    /**
     * @brief 获取最后一次成功插入的 主键 Id
     * @param db 
     * @return auto 
     */
    auto getLastInsertPrimaryKeyId() const noexcept {
        return ::sqlite3_last_insert_rowid(::sqlite3_db_handle(_stmt));;
    }

    /**
     * @brief 重置 stmt
     */
    void reset() {
        if (::sqlite3_reset(_stmt) != SQLITE_OK) [[unlikely]] {
            throw std::runtime_error{"reset: " + getErrMsg()};
        }
    }

    /**
     * @brief 清除绑定值
     */
    void clearBind() {
        if (::sqlite3_clear_bindings(_stmt) != SQLITE_OK) [[unlikely]] {
            throw std::runtime_error{"clearBind: " + getErrMsg()};
        }
    }

    /**
     * @brief 获取错误字符串
     * @return std::string 
     */
    std::string getErrMsg() const noexcept {
        return ::sqlite3_errmsg(::sqlite3_db_handle(_stmt));
    }

    template <typename U>
    U getColumnByIndex(std::size_t index) {
        using T = RemovePrimaryKeyType<U>;
        if constexpr (std::is_integral_v<T>) {
            return U{static_cast<T>(::sqlite3_column_int64(_stmt, static_cast<int>(index)))};
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(::sqlite3_column_double(_stmt, static_cast<int>(index)));
        } else if constexpr (meta::StringType<T> || isSQLiteSqlTypeVal<T>) {
            auto* str = reinterpret_cast<const char *>(
                ::sqlite3_column_text(_stmt, static_cast<int>(index))
            );
            auto len = ::sqlite3_column_bytes(_stmt, static_cast<int>(index));
            if constexpr (isSQLiteSqlTypeVal<T>) {
                return SQLiteSqlType<T>::columnType({str, static_cast<std::size_t>(len)});
            } else {
                return T{str, static_cast<std::size_t>(len)};
            }
        } else {
            // 不支持该类型
            static_assert(!sizeof(T), "type is not sql type");
        }
    }

    operator ::sqlite3_stmt*() noexcept {
        return _stmt;
    }

    SQLiteStmt& operator=(SQLiteStmt&) = delete;
    SQLiteStmt& operator=(SQLiteStmt&& that) noexcept {
        if (this == std::addressof(that)) {
            return *this;
        }
        std::swap(that._stmt, _stmt);
        return *this;
    }

    ~SQLiteStmt() noexcept {
        if (_stmt) [[likely]] {
            ::sqlite3_finalize(_stmt);
        }
    }
private:
    ::sqlite3_stmt* _stmt;
};

} // namespace HX::db