// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/assertion.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/concepts.h"
#include "potato/spud/rc.h"
#include "potato/spud/typelist.h"
#include "potato/spud/utility.h"
#include "potato/spud/zstring_view.h"

#include <tuple>

extern "C" {
    struct sqlite3;
    struct sqlite3_stmt;
}

namespace up {
    inline namespace posql {
        enum class SqlResult { Ok = 0, Error };

        namespace sqlutil {
            [[nodiscard]] sqlite3_stmt* compile(sqlite3* db, string_view sql);
            [[nodiscard]] bool isComplete(sqlite3_stmt* stmt) noexcept;
            void nextRow(sqlite3_stmt* stmt) noexcept;
            void reset(sqlite3_stmt* stmt) noexcept;
            void destroy(sqlite3_stmt* stmt) noexcept;

            [[nodiscard]] int64 fetchColumnI64(sqlite3_stmt* stmt, int index) noexcept;
            [[nodiscard]] zstring_view fetchColumnString(sqlite3_stmt* stmt, int index) noexcept;

            void bindParam(sqlite3_stmt* stmt, int index, int64 value) noexcept;
            void bindParam(sqlite3_stmt* stmt, int index, string_view value) noexcept;
            void bindParam(sqlite3_stmt* stmt, int index, UUID const& value) noexcept;
            template <enumeration E>
            void bindParam(sqlite3_stmt* stmt, int index, E value) noexcept {
                bindParam(stmt, index, to_underlying(value));
            }

            template <typename T>
            auto fetchColumn(sqlite3_stmt* stmt, int index) {
                if constexpr (std::is_same_v<T, UUID>) {
                    return UUID::fromString(fetchColumnString(stmt, index));
                }
                else if constexpr (std::is_constructible_v<T, int64>) {
                    return fetchColumnI64(stmt, index);
                }
                else if constexpr (std::is_enum_v<T>) {
                    return T(fetchColumnI64(stmt, index));
                }
                else if constexpr (std::is_constructible_v<T, zstring_view>) {
                    return fetchColumnString(stmt, index);
                }
            }

            template <int... I, typename... T>
            std::tuple<T...> fetchColumns(sqlite3_stmt* stmt, std::integer_sequence<int, I...>, typelist<T...>) {
                return std::tuple<T...>(fetchColumn<T>(stmt, I)...);
            }

            template <typename... T>
            std::tuple<T...> fetchColumns(sqlite3_stmt* stmt) {
                return fetchColumns(stmt, std::make_integer_sequence<int, sizeof...(T)>{}, typelist<T...>{});
            }

            template <int... I, typename... T>
            void bindParams(sqlite3_stmt* stmt, std::integer_sequence<int, I...>, T const&... args) noexcept {
                (((void)bindParam(stmt, I, args)), ...);
            }

            template <typename... T>
            void bindParams(sqlite3_stmt* stmt, T const&... args) noexcept {
                if constexpr (sizeof...(T) != 0) {
                    bindParams(stmt, std::make_integer_sequence<int, sizeof...(args)>{}, args...);
                }
            }

            struct Stmt : shared<Stmt> {
                explicit Stmt(sqlite3_stmt* s) noexcept : stmt(s) { }
                virtual ~Stmt() { destroy(stmt); }
                sqlite3_stmt* stmt = nullptr;
            };

        } // namespace sqlutil

        class Database;
        class Statement;
        class Transaction;

        template <typename...>
        class Query;
        struct QuerySentinel { };
        template <typename...>
        class Cursor;

        template <typename... T>
        class Query {
        public:
            using value_type = std::tuple<T...>;
            using size_type = std::size_t;
            using iterator = Cursor<T...>;
            using sentinel = QuerySentinel;

            iterator begin() noexcept { return iterator(std::move(_stmt)); }
            sentinel end() noexcept { return sentinel{}; }

        private:
            explicit Query(rc<sqlutil::Stmt> stmt) : _stmt(std::move(stmt)) { }

            rc<sqlutil::Stmt> _stmt;

            friend Statement;
            friend Database;
        };

        class Database {
        public:
            Database() noexcept = default;
            ~Database() noexcept { close(); }

            Database(Database const&) = delete;
            Database& operator=(Database const&) = delete;

            [[nodiscard]] SqlResult open(zstring_view file_name) noexcept;
            void close() noexcept;

            [[nodiscard]] bool empty() const noexcept { return _conn == nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return _conn != nullptr; }

            [[nodiscard]] inline Statement prepare(zstring_view sql);

            template <typename... T>
            [[nodiscard]] SqlResult execute(string_view sql, T const&... args) {
                UP_ASSERT(_conn != nullptr);
                sqlite3_stmt* stmt = compile(sql, args...);
                SqlResult const result = stmt ? SqlResult::Ok : SqlResult::Error;
                sqlutil::destroy(stmt);
                return result;
            }

            template <typename... R, typename... T>
            [[nodiscard]] Query<R...> query(string_view sql, T const&... args) {
                UP_ASSERT(_conn != nullptr);
                return Query<R...>{new_shared<sqlutil::Stmt>(compile(sql, args...))};
            }

            template <typename... R, typename... T>
            [[nodiscard]] auto queryOne(string_view sql, T const&... args) {
                UP_ASSERT(_conn != nullptr);
                sqlutil::Stmt stmt(compile(sql, args...));
                auto const rs = sqlutil::fetchColumns<R...>(stmt.stmt);
                return rs;
            }

            [[nodiscard]] Transaction begin() noexcept;

        private:
            template <typename... T>
            sqlite3_stmt* compile(string_view sql, T const&... args) {
                sqlite3_stmt* stmt = sqlutil::compile(_conn, sql);
                if (stmt != nullptr) {
                    sqlutil::bindParams(stmt, args...);
                    sqlutil::nextRow(stmt);
                }
                return stmt;
            }

            sqlite3* _conn = nullptr;
        };

        class Transaction {
        public:
            ~Transaction() { rollback(); }

            Transaction(Transaction&& rhs) noexcept : _conn(rhs._conn) { rhs._conn = nullptr; }
            Transaction& operator=(Transaction&& rhs) noexcept {
                commit();
                _conn = rhs._conn;
                rhs._conn = nullptr;
                return *this;
            }

            void commit();
            void rollback();

        private:
            explicit Transaction(sqlite3* conn) noexcept : _conn(conn) { }

            friend Database;

            sqlite3* _conn = nullptr;
        };

        class Statement {
        public:
            Statement() noexcept = default;
            explicit Statement(rc<sqlutil::Stmt> stmt) noexcept : _stmt(std::move(stmt)) { }

            Statement(Statement&& rhs) noexcept : _stmt(std::move(rhs._stmt)) { rhs._stmt = nullptr; }
            Statement& operator=(Statement&& rhs) noexcept;

            [[nodiscard]] bool empty() const noexcept { return _stmt == nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return _stmt != nullptr; }

            template <typename... T>
            [[nodiscard]] SqlResult execute(T const&... args) noexcept {
                sqlutil::reset(_stmt->stmt);
                sqlutil::bindParams(_stmt->stmt, args...);
                sqlutil::nextRow(_stmt->stmt);
                return SqlResult::Ok;
            }

            template <typename... R, typename... T>
            [[nodiscard]] Query<R...> query(T const&... args) noexcept {
                sqlutil::reset(_stmt->stmt);
                sqlutil::bindParams(_stmt->stmt, args...);
                sqlutil::nextRow(_stmt->stmt);
                return Query<R...>{_stmt};
            }

            template <typename... R, typename... T>
            [[nodiscard]] auto queryOne(T const&... args) noexcept {
                sqlutil::reset(_stmt->stmt);
                sqlutil::bindParams(_stmt->stmt, args...);
                sqlutil::nextRow(_stmt->stmt);
                auto result = sqlutil::fetchColumns<R...>(_stmt->stmt);
                sqlutil::reset(_stmt->stmt);
                return result;
            }

        private:
            template <typename...>
            friend class Query;
            template <typename...>
            friend class Cursor;

            rc<sqlutil::Stmt> _stmt;
        };

        template <typename... T>
        class Cursor {
        public:
            [[nodiscard]] bool operator==(QuerySentinel) noexcept {
                return _stmt == nullptr || sqlutil::isComplete(_stmt->stmt);
            }
            Cursor& operator++() noexcept {
                sqlutil::nextRow(_stmt->stmt);
                return *this;
            }
            [[nodiscard]] inline std::tuple<T...> operator*() { return sqlutil::fetchColumns<T...>(_stmt->stmt); }

        private:
            explicit Cursor(rc<sqlutil::Stmt> stmt) : _stmt(std::move(stmt)) { }

            rc<sqlutil::Stmt> _stmt;

            template <typename...>
            friend class Query;
        };

        Statement Database::prepare(zstring_view sql) {
            return Statement(new_shared<sqlutil::Stmt>(sqlutil::compile(_conn, sql)));
        }
    }; // namespace posql

} // namespace up
