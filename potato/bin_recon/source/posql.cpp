// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "posql.h"

#include "potato/runtime/assertion.h"

#include <sqlite3.h>

up::SqlResult up::Database::open(zstring_view file_name) noexcept {
    close();

    auto const rs = sqlite3_open(file_name.c_str(), &_conn);
    if (rs != SQLITE_OK) {
        return SqlResult::Error;
    }

    return SqlResult::Ok;
}

void up::Database::close() noexcept {
    sqlite3_close(_conn);
    _conn = nullptr;
}

up::Transaction up::Database::begin() noexcept {
    UP_ASSERT(_conn != nullptr);
    if (execute("BEGIN") != SqlResult::Ok) {
        return Transaction(nullptr);
    }
    return Transaction(_conn);
}

void up::Transaction::commit() {
    if (_conn != nullptr) {
        sqlite3_exec(_conn, "COMMIT", nullptr, nullptr, nullptr);
        _conn = nullptr;
    }
}

void up::Transaction::rollback() {
    if (_conn != nullptr) {
        sqlite3_exec(_conn, "ROLLBACK", nullptr, nullptr, nullptr);
        _conn = nullptr;
    }
}

up::Statement& up::Statement::operator=(Statement&& rhs) noexcept {
    if (_stmt != rhs._stmt) {
        _stmt = std::move(rhs._stmt);
    }
    return *this;
}

auto up::posql::sqlutil::compile(sqlite3* db, string_view sql) -> sqlite3_stmt* {
    UP_ASSERT(db != nullptr);
    UP_ASSERT(!sql.empty());

    sqlite3_stmt* stmt = nullptr;
    [[maybe_unused]] auto const rs =
        sqlite3_prepare_v3(db, sql.data(), static_cast<int>(sql.size()), 0, &stmt, nullptr);
    UP_ASSERT(rs == SQLITE_OK, "{}: {}", rs, sqlite3_errmsg(db));
    return stmt;
}

void up::posql::sqlutil::destroy(sqlite3_stmt* stmt) noexcept {
    if (stmt != nullptr) {
        sqlite3_finalize(stmt);
    }
}

bool up::posql::sqlutil::isComplete(sqlite3_stmt* stmt) noexcept {
    UP_GUARD(stmt != nullptr, true);
    return sqlite3_stmt_busy(stmt) == 0;
}

void up::posql::sqlutil::nextRow(sqlite3_stmt* stmt) noexcept {
    UP_GUARD_VOID(stmt != nullptr);
    for (;;) {
        auto const rs = sqlite3_step(stmt);
        if (rs == SQLITE_ROW || rs == SQLITE_DONE) {
            break;
        }
        if (rs == SQLITE_BUSY) {
            continue;
        }
        sqlite3_reset(stmt);
        break;
    }
}

void up::posql::sqlutil::reset(sqlite3_stmt* stmt) noexcept {
    UP_GUARD_VOID(stmt != nullptr);
    sqlite3_reset(stmt);
}

up::int64 up::posql::sqlutil::fetchColumnI64(sqlite3_stmt* stmt, int index) noexcept {
    UP_GUARD(stmt != nullptr, 0u);
    return sqlite3_column_int64(stmt, index);
}

up::zstring_view up::posql::sqlutil::fetchColumnString(sqlite3_stmt* stmt, int index) noexcept {
    UP_GUARD(stmt != nullptr, zstring_view{});
    unsigned char const* const text = sqlite3_column_text(stmt, index);
    return text != nullptr ? zstring_view{reinterpret_cast<char const*>(text)} : ""_zsv;
}

void up::posql::sqlutil::bindParam(sqlite3_stmt* stmt, int index, int64 value) noexcept {
    UP_GUARD_VOID(stmt != nullptr);
    sqlite3_bind_int64(stmt, index + 1, value);
}

void up::posql::sqlutil::bindParam(sqlite3_stmt* stmt, int index, string_view value) noexcept {
    UP_GUARD_VOID(stmt != nullptr);
    sqlite3_bind_text(stmt, index + 1, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT); // NOLINT
}

void up::posql::sqlutil::bindParam(sqlite3_stmt* stmt, int index, UUID const& value) noexcept {
    UP_GUARD_VOID(stmt != nullptr);
    char uuidStr[UUID::strLength] = {0};
    nanofmt::format_to(uuidStr, "{}", value);

    sqlite3_bind_text(stmt, index + 1, uuidStr, -1, SQLITE_TRANSIENT); // NOLINT
}
