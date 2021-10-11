// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

// Based on Howard Hinnant's "Types Don't Know #"

#pragma once

#include "hash_fnv1a.h"
#include "int_types.h"
#include "traits.h"

namespace up {
    struct default_hash;

    template <typename HashAlgorithm = default_hash>
    struct uhash;

    template <typename HashAlgorithm = default_hash, typename T>
    constexpr auto hash_value(T const& value) -> typename HashAlgorithm::result_type;

    template <typename HashAlgorithm = default_hash, size_t N>
    constexpr auto hash_value(char const (&value)[N]) -> typename HashAlgorithm::result_type;

    template <typename Hash>
    constexpr auto hash_combine(Hash left, Hash right) noexcept -> Hash;

    template <typename HashAlgorithm, typename Value>
    struct hash_result;
    template <typename HashAlgorithm, typename Value>
    using hash_result_t = typename hash_result<HashAlgorithm, Value>::type;

    template <typename>
    class vector;
} // namespace up

namespace up {
    template <typename HashAlgorithm, typename T>
    HashAlgorithm& hash_append(HashAlgorithm& hasher, T const& value) noexcept requires is_contiguous_v<T> {
        // NOLINTNEXTLINE(bugprone-sizeof-expression)
        hasher.append_bytes(reinterpret_cast<char const*>(&value), sizeof(value));
        return hasher;
    }
} // namespace up

struct up::default_hash {
    using result_type = typename fnv1a::result_type;

    constexpr void append_bytes(char const* data, size_t size) noexcept { _fnva1.append_bytes(data, size); }

    constexpr auto finalize() const noexcept {
        auto result = _fnva1.finalize();
#if defined(_MSC_VER)
        // Microsoft's std::hash implementation does this, so let's be compatible
        // TODO: what does libc++ do?
        // TODO: should this all just forward to std::hash instead of our own hasher?
        result ^= (result >> (4 * sizeof(void*)));
#endif
        return result;
    }

private:
    fnv1a _fnva1;
};

template <typename HashAlgorithm>
struct up::uhash {
    using result_type = typename HashAlgorithm::result_type;

    template <typename T>
    constexpr result_type operator()(T&& value) const {
        return hash_value(value);
    }
};

template <typename HashAlgorithm, typename T>
constexpr auto up::hash_value(T const& value) -> typename HashAlgorithm::result_type {
    HashAlgorithm hasher{};
    using up::hash_append;
    hash_append(hasher, value);
    return hasher.finalize();
}

template <typename HashAlgorithm, size_t N>
constexpr auto up::hash_value(char const (&value)[N]) -> typename HashAlgorithm::result_type {
    HashAlgorithm hasher{};
    hasher.append_bytes(value, N - 1 /*NUL byte typically present in character literals*/);
    return hasher.finalize();
}

template <typename Hash>
constexpr auto up::hash_combine(Hash left, Hash right) noexcept -> Hash {
    left ^= right + 0x9e3779b9 + (left << 6) + (left >> 2);
    return left;
}

template <typename HashAlgorithm, typename Value>
struct up::hash_result {
    using type = std::remove_cvref_t<decltype(std::declval<HashAlgorithm>()(std::declval<Value>()))>;
};
