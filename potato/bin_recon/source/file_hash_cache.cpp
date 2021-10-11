// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "file_hash_cache.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/out_ptr.h"

up::FileHashCache::FileHashCache() = default;

up::FileHashCache::~FileHashCache() = default;

auto up::FileHashCache::hashAssetContent(span<up::byte const> contents) noexcept -> up::uint64 {
    return hash_value<fnv1a>(contents);
}

auto up::FileHashCache::hashAssetStream(Stream& stream) -> up::uint64 {
    constexpr int block_size = 8 * 1024;

    auto hasher = fnv1a();
    up::byte buffer[block_size];
    while (!stream.isEof()) {
        span<up::byte> read(buffer, sizeof(buffer));
        stream.read(read);
        if (read.empty()) {
            break;
        }
        hash_append(hasher, read);
    }
    return static_cast<uint64>(hasher.finalize());
}

auto up::FileHashCache::hashAssetAtPath(zstring_view path) -> up::uint64 {
    auto const [rs, stat] = fs::fileStat(path);
    if (rs != IOResult::Success) {
        return 0;
    }

    auto const pathHash = hash_value(path);

    auto item = _hashes.find(pathHash);
    if (item) {
        if (rs == IOResult::Success && stat.size == item->value.size && stat.mtime == item->value.mtime) {
            return item->value.contentHash;
        }
    }

    auto fstream = fs::openRead(path);
    auto const contentHash = hashAssetStream(fstream);

    (void)_addEntryStmt.execute(path, contentHash, stat.mtime, stat.size);

    _hashes.insert(
        pathHash,
        {.pathHash = pathHash, .contentHash = contentHash, .mtime = stat.mtime, .size = stat.size});

    return contentHash;
}

bool up::FileHashCache::close() {
    _conn.close();
    return true;
}

bool up::FileHashCache::open(zstring_view cache_path) {
    _conn.close();

    auto const rs = _conn.open(cache_path);
    if (rs != SqlResult::Ok) {
        return false;
    }

    // ensure cache table exists
    if (_conn.execute("CREATE TABLE IF NOT EXISTS hash_cache (os_path STRING PRIMARY KEY, hash INTEGER, mtime INTEGER, "
                      "size INTEGER)") != SqlResult::Ok) {
        _conn.close();
        return false;
    }

    // create cached add entry statement for later use
    _addEntryStmt = _conn.prepare(
        "INSERT INTO hash_cache (os_path, hash, mtime, size) VALUES(?, ?, ?, ?) "
        "ON CONFLICT (os_path) DO UPDATE SET hash=excluded.hash, mtime=excluded.mtime, size=excluded.size");

    // load cache entries
    auto stmt = _conn.prepare("SELECT os_path, hash, mtime, size FROM hash_cache");

    for (auto const& [path, contentHash, mtime, size] : stmt.query<zstring_view, uint64, uint64, uint64>()) {
        auto rec_ptr = new_box<HashRecord>();
        auto& rec = *rec_ptr;
        rec.pathHash = hash_value(path);
        rec.contentHash = contentHash;
        rec.mtime = mtime;
        rec.size = size;
    }

    return true;
}
