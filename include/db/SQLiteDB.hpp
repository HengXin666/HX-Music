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

#include <meta/StaticConstexpr.hpp>
#include <meta/FixedString.hpp>

#include <HXLibs/reflection/MemberName.hpp>
#include <HXLibs/reflection/EnumName.hpp>
#include <HXLibs/reflection/TypeName.hpp>
#include <HXLibs/log/serialize/ToString.hpp>

#include <db/SQLiteMeta.hpp>
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

constexpr auto SqlOpCnt = reflection::getValidEnumValueCnt<SqlOp>();

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
    } else if constexpr (meta::StringType<T> || isSQLiteSqlTypeVal<T>) {
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

#if 0

constexpr bool isSpace(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

constexpr bool isPunct(char c) noexcept {
    // https://en.cppreference.com/w/cpp/string/byte/isalnum
    constexpr auto arr = meta::StaticConstexpr<decltype([]{
        std::array<bool, 127> arr{};
        for (char c : R"(!"#$%&'()*+,-./ 	:;<=>?@[\]^_`{|}~)") {
            arr[c] = true;
        }
        return arr;
    })>::get();
    return c >= 0 && arr[c];
}

constexpr bool isAlnumOrUnderscore(char c) noexcept {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
            c == '_';
}

/**
 * @brief 解析 SQL 的占位符对应的字段名称
 * @param sql
 * @return std::vector<std::string_view>
 */
template <meta::FixedString Sql>
inline constexpr auto extractFields() {
    constexpr auto view = meta::ToCharPack<Sql>::view();
    constexpr auto len = Sql.size();

    // 先扫描问号数量
    constexpr std::size_t MaxFields = [&]() constexpr {
        std::size_t count = 0;
        for (std::size_t i = 0; i < len; ++i) {
            count += view[i] == '?';
        }
        return count;
    }();

    std::array<std::string_view, MaxFields> fields{};
    std::size_t found = 0;

    for (std::size_t pos = 0; pos < len; ++pos) {
        if (view[pos] != '?') {
            continue;
        }

        // 向前找到字段名末尾 (第一个非空格非标点)
        std::size_t r = pos;
        while (r > 0 && (isSpace(view[r - 1]) || isPunct(view[r - 1]))) {
            --r;
        }

        // 再向前找到字段名起点 (非字母数字且非下划线)
        std::size_t l = r;
        while (l > 0 && (isAlnumOrUnderscore(view[l - 1]))) {
            --l;
        }

        if (r > l) {
            fields[found++] = view.substr(l, r - l);
        }
    }

    // 截断数组
    std::array<std::string_view, MaxFields> res{};
    for (std::size_t i = 0; i < found; ++i) {
        res[i] = fields[i];
    }
    return res;
}

template <typename T>
class SqlCallChain {
public:
    SqlCallChain(std::string sql)
        : _sql{std::move(sql)}
    {}

    template <meta::FixedString Str>
    constexpr SqlCallChain& where() {
        if (_opEd & SqlOp::Where) [[unlikely]] {
            throw std::runtime_error{"@todo"};
        }
        _opEd |= SqlOp::Where;

        [[maybe_unused]] constexpr auto fields = extractFields<Str>();

        constexpr auto nameMap = reflection::getMembersNamesMap<T>();

        [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            constexpr auto tp = reflection::internal::getStaticObjPtrTuple<T>();
            (([&] <std::size_t Idx> (std::index_sequence<Idx>) {
                constexpr auto I = nameMap.at(fields[Idx]);
                using Type = decltype(*std::get<I>(tp));
                // 完全没必要... 仅仅只是为了编译时判断类型吗? 没用
            }(std::index_sequence<Is>{})), ...);
        } (std::make_index_sequence<fields.size()>{});

        _sql += meta::ToCharPack<Str>::view();
        return *this;
    }
private:
    std::string _sql;
    uint8_t _opEd;
    std::size_t _idx = 0;
};

#endif

struct [[nodiscard]] StmtCallChain {
    StmtCallChain(std::string_view sql, ::sqlite3* db)
        : _db{db}
        , _stmt{sql, _db}
    {}

    template <typename... Args>
    StmtCallChain& bind(Args&&... args) noexcept {
        auto tp = std::make_tuple(std::forward<Args>(args)...);
        [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            (([&] <std::size_t Idx> (std::index_sequence<Idx>) {
                auto& t = std::get<Idx>(tp);
                using T = RemovePrimaryKeyType<meta::remove_cvref_t<decltype(t)>>;
                if constexpr (std::is_integral_v<T>) {
                    ::sqlite3_bind_int64(_stmt, Idx + 1, t); 
                } else if constexpr (std::is_floating_point_v<T>) {
                    ::sqlite3_bind_double(_stmt, Idx + 1, t); 
                } else if constexpr (meta::StringType<T> || isSQLiteSqlTypeVal<T>) {
                    if constexpr (isSQLiteSqlTypeVal<T>) {
                        auto str = SQLiteSqlType<T>::bind(t);
                        ::sqlite3_bind_text(_stmt, Idx + 1, str.data(), str.size(), SQLITE_TRANSIENT);
                    } else {
                        ::sqlite3_bind_text(_stmt, Idx + 1, t.data(), t.size(), SQLITE_TRANSIENT);
                    }
                } else {
                    // 不支持该类型
                    static_assert(!sizeof(T), "type is not sql type");
                }
            }(std::index_sequence<Is>{})), ...);
        } (std::make_index_sequence<std::tuple_size_v<decltype(tp)>>{});
        return *this;
    }

    // 执行
    bool exec() const noexcept {
        return _stmt.step() == SQLITE_DONE;
    }

    // 带异常的
    void execOnThrow() const {
        if (!exec()) [[unlikely]] {
            throw std::runtime_error{"SQL Error: " + std::string{::sqlite3_errmsg(_db)}};
        }
    }
private:
    ::sqlite3* _db;
    SQLiteStmt _stmt;
};

} // namespace internal

class SQLiteDB {
    void exec(std::string_view sql) const {
        internal::execSql(sql, _db);
    }
public:
    SQLiteDB() : _db{} {}

    SQLiteDB(std::string_view filePath) 
        : SQLiteDB{}
    {
        if (::sqlite3_open(filePath.data(), &_db) != SQLITE_OK) [[unlikely]] {
            throw std::runtime_error{
                "Failed to open database: " + std::string{::sqlite3_errmsg(_db)}
            };
        }
    }

    SQLiteDB(SQLiteDB const&) = delete;
    SQLiteDB(SQLiteDB&& that) noexcept 
        : _db{that._db}
    {
        that._db = nullptr;
    }

    SQLiteDB& operator=(SQLiteDB const&) noexcept = delete;
    SQLiteDB& operator=(SQLiteDB&& that) noexcept {
        std::swap(_db, that._db);
        return *this;
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
            using U = meta::remove_cvref_t<decltype(val)>;
            if constexpr (Idx > 0) {
                sql += ", ";
            }
            sql += name;
            sql += ' ';
            if constexpr (isPrimaryKeyVal<U>) {
                sql += internal::getSqlTypeStr<typename U::PrimaryKeyType>();
                sql += " PRIMARY KEY";
            } else {
                sql += internal::getSqlTypeStr<U>();
            }
        });
        sql += ");";
        exec(sql);
    }

    template <typename T>
    auto insert(T&& t) const {
        using U = meta::remove_cvref_t<T>;
        std::string sql = "INSERT INTO ";
        sql += reflection::getTypeName<U>();
        sql += " (";
        auto obj = reflection::internal::getStaticObj<U>();
        reflection::forEach(obj, [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto&& val
        ) {
            using Type = meta::remove_cvref_t<decltype(val)>;
            if constexpr (isPrimaryKeyVal<Type>) {
                // 主键不会进行指定
                return;
            } else {
                sql += name;
                sql += ',';
            }
        });
        sql.pop_back();
        sql += ") VALUES (";
        reflection::forEach(std::forward<T>(t), [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto& val
        ) {
            using Type = meta::remove_cvref_t<decltype(val)>;
            if constexpr (isPrimaryKeyVal<Type>) {
                // 主键不会进行指定
                return;
            } else {
                using ValType = RemovePrimaryKeyType<Type>;
                if constexpr (std::is_arithmetic_v<ValType>) {
                    log::toString(val, sql);
                } else if constexpr (meta::StringType<ValType> || isSQLiteSqlTypeVal<ValType>) {
                    sql += '\'';
                    if constexpr (isSQLiteSqlTypeVal<ValType>) {
                        sql += SQLiteSqlType<ValType>::bind(val);
                    } else {
                        log::toString(static_cast<const char*>(val.data()), sql);
                    }
                    sql += '\'';
                } else {
                    // 不支持该类型
                    static_assert(!sizeof(T), "type is not sql type");
                }
                sql += ',';
            }
        });
        sql.pop_back();
        sql += ");";
        SQLiteStmt stmt(sql, _db);
        if (stmt.step() == SQLITE_DONE) [[likely]] {
            if constexpr (requires {
                GetFirstPrimaryKeyType<T> {};
            }) {
                return static_cast<GetFirstPrimaryKeyType<T>>(
                    stmt.getLastInsertPrimaryKeyId(_db)
                );
            } else {
                return stmt.getLastInsertPrimaryKeyId(_db);
            }
        } else [[unlikely]] {
            throw std::runtime_error{"Insert failed: " + std::string{::sqlite3_errmsg(_db)}};
        }
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
    internal::StmtCallChain deleteBy(std::string sqlBody) {
        std::string sql = "DELETE FROM ";
        sql += reflection::getTypeName<T>();
        sql += ' ';
        sql += std::move(sqlBody);
        return {sql, _db};
    }

    template <typename T>
    internal::StmtCallChain updateBy(T&& t, std::string sqlBody) {
        std::string sql = "UPDATE ";
        sql += reflection::getTypeName<meta::remove_cvref_t<T>>();
        sql += " SET ";
        reflection::forEach(std::forward<T>(t), [&] <std::size_t Idx> (
            std::index_sequence<Idx>, std::string_view name, auto& val
        ) {
            using Type = meta::remove_cvref_t<decltype(val)>;
            if constexpr (isPrimaryKeyVal<Type>) {
                // 主键不会进行指定
                return;
            } else {
                using ValType = RemovePrimaryKeyType<Type>;
                sql += name;
                sql += "=";
                if constexpr (std::is_arithmetic_v<ValType>) {
                    log::toString(val, sql);
                } else if constexpr (meta::StringType<ValType> || isSQLiteSqlTypeVal<ValType>) {
                    sql += '\'';
                    if constexpr (isSQLiteSqlTypeVal<ValType>) {
                        sql += SQLiteSqlType<ValType>::bind(val);
                    } else {
                        log::toString(static_cast<const char*>(val.data()), sql);
                    }
                    sql += '\'';
                } else {
                    // 不支持该类型
                    static_assert(!sizeof(T), "type is not sql type");
                }
                sql += ',';
            }
        });
        sql.back() = ' ';
        sql += std::move(sqlBody);
        return {sql, _db};
    }

private:
    ::sqlite3* _db{};
};

[[nodiscard]] inline SQLiteDB open(std::string_view filePath) {
    return SQLiteDB{filePath};
}

} // namespace HX::db