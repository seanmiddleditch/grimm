// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/posql/posql.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_map.h"
#include "potato/spud/int_types.h"
#include "potato/spud/span.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class FileHashCache {
    public:
        FileHashCache();
        ~FileHashCache();

        FileHashCache(FileHashCache const&) = delete;
        FileHashCache& operator=(FileHashCache const&) = delete;

        static uint64 hashAssetContent(span<byte const> contents) noexcept;
        static uint64 hashAssetStream(Stream& stream);

        uint64 hashAssetAtPath(zstring_view path);

        bool open(zstring_view cache_path);
        bool close();

    private:
        struct HashRecord {
            uint64 pathHash = 0;
            uint64 contentHash = 0;
            uint64 mtime = 0;
            uint64 size = 0;
        };

        hash_map<uint64, HashRecord, identity> _hashes;
        Database _conn;
        Statement _addEntryStmt;
    };
} // namespace up
