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

#include <HXLibs/meta/ContainerConcepts.hpp>

namespace HX::db {

class SQLiteStmt {
public:
    explicit SQLiteStmt(std::string_view sql, ::sqlite3* db)
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

    int step() const noexcept {
        return ::sqlite3_step(_stmt);
    }

    template <typename T>
    T getColumnByIndex(std::size_t index) {
        if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(::sqlite3_column_int64(_stmt, static_cast<int>(index)));
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(::sqlite3_column_double(_stmt, static_cast<int>(index)));
        } else if constexpr (meta::StringType<T>) {
            return T{reinterpret_cast<const char *>(::sqlite3_column_text(_stmt, static_cast<int>(index)))};
        } else {
            // 不支持该类型
            static_assert(!sizeof(T), "type is not sql type");
        }
    }

    operator ::sqlite3_stmt*() noexcept {
        return _stmt;
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