// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "uuid.h"

#include "potato/spud/int_types.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up {
    /// @brief Mapping of resource identifiers to CAS hashes and filenames
    class ResourceManifest {
    public:
        using Id = uint64;

        struct Record {
            UUID uuid = {};
            Id logicalId = {};
            uint64 hash = 0;
            string logicalName;
            string filename;
            string type;
        };

        static constexpr zstring_view columnUuid = "UUID"_zsv;
        static constexpr zstring_view columnLogicalId = "LOGICAL_ID"_zsv;
        static constexpr zstring_view columnLogicalName = "LOGICAL_NAME"_zsv;
        static constexpr zstring_view columnContentType = "CONTENT_TYPE"_zsv;
        static constexpr zstring_view columnContentHash = "CONTENT_HASH"_zsv;
        static constexpr zstring_view columnDebugName = "DEBUG_NAME"_zsv;
        static constexpr int version = 3;

        void clear() { _records.clear(); }
        auto size() const noexcept { return _records.size(); }

        view<Record> records() const noexcept { return _records; }

        Record const* findRecord(Id logicalId) const noexcept {
            for (Record const& record : _records) {
                if (record.logicalId == logicalId) {
                    return &record;
                }
            }
            return nullptr;
        }

        UP_RUNTIME_API static bool parseManifest(string_view input, ResourceManifest& manifest);

    private:
        vector<Record> _records;
    };
} // namespace up
