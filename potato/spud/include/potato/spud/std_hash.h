// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

// Based on Howard Hinnant's "Types Don't Know #"

#pragma once

#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace up {
    template <typename HashAlgorithm, typename CharT, typename CharTraits, typename AllocatorT>
    inline void hash_append(HashAlgorithm& hasher, std::basic_string<CharT, CharTraits, AllocatorT> const& string) {
        hasher.append_bytes(string.data(), string.size());
    }

    template <typename HashAlgorithm, typename FirstT, typename SecondT>
    inline void hash_append(HashAlgorithm& hasher, std::pair<FirstT, SecondT> const& pair) {
        hash_append(hasher, pair.first);
        hash_append(hasher, pair.second);
    }

    template <typename HashAlgorithm>
    inline void hash_append(HashAlgorithm& hasher, std::type_index type) {
        // hash_code might not be the same number of bits as the hasher so we rehash.
        // we rehash even if they are the same number of bits, which is probably not ideal, but works.
        hash_append(hasher, type.hash_code());
    }
} // namespace up
