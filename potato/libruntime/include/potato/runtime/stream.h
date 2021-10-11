// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "io_result.h"

#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/span.h"
#include "potato/spud/string_view.h"

namespace up {
    class string;
    class string_view;
    template <typename>
    class vector;

    class Stream {
    public:
        using size_type = uint64;
        using difference_type = int64;

        enum class Seek { Begin, End, Current };

        class Backend {
        public:
            Backend() = default;
            virtual ~Backend() = default;

            Backend(Backend const&) = delete;
            Backend& operator=(Backend const&) = delete;

            virtual bool isOpen() const noexcept = 0;
            virtual bool isEof() const noexcept = 0;
            virtual bool canRead() const noexcept = 0;
            virtual bool canWrite() const noexcept = 0;
            virtual bool canSeek() const noexcept = 0;

            virtual IOResult seek(Seek position, difference_type offset) = 0;
            virtual difference_type tell() const = 0;
            virtual difference_type remaining() const = 0;

            virtual IOResult read(span<byte>& buffer) = 0;
            virtual IOResult write(span<byte const> buffer) = 0;
            virtual IOResult flush() = 0;
        };

        Stream() = default;
        Stream(box<Backend> impl) noexcept : _impl(std::move(impl)) {}
        Stream(std::nullptr_t) noexcept {}

        Stream& operator=(box<Backend> impl) noexcept {
            _impl = std::move(impl);
            return *this;
        }
        Stream& operator=(std::nullptr_t) noexcept {
            _impl.reset();
            return *this;
        }

        bool isOpen() const noexcept { return _impl != nullptr && _impl->isOpen(); }
        bool isEof() const noexcept { return _impl == nullptr || _impl->isEof(); }
        bool canRead() const noexcept { return _impl != nullptr && _impl->canRead(); }
        bool canWrite() const noexcept { return _impl != nullptr && _impl->canWrite(); }
        bool canSeek() const noexcept { return _impl != nullptr && _impl->canSeek(); }

        explicit operator bool() const noexcept { return isOpen(); }

        IOResult seek(Seek position, difference_type offset) { return _impl->seek(position, offset); }
        difference_type tell() const { return _impl->tell(); }
        difference_type remaining() const { return _impl->remaining(); }

        IOResult read(span<byte>& buffer) { return _impl->read(buffer); }
        IOResult write(span<byte const> buffer) { return _impl->write(buffer); }
        IOResult flush() { return _impl->flush(); }

        void write(string_view text) { _impl->write({reinterpret_cast<byte const*>(text.data()), text.size()}); }

        void close() noexcept { _impl.reset(); }

    private:
        box<Backend> _impl;
    };

    [[nodiscard]] UP_RUNTIME_API auto readBinary(Stream& stream, vector<up::byte>& out) -> IOResult;
    [[nodiscard]] UP_RUNTIME_API auto readBinary(Stream& stream) -> IOReturn<vector<up::byte>>;
    [[nodiscard]] UP_RUNTIME_API auto readText(Stream& stream, string& out) -> IOResult;
    [[nodiscard]] UP_RUNTIME_API auto readText(Stream& stream) -> IOReturn<string>;

    [[nodiscard]] UP_RUNTIME_API auto writeAllText(Stream& stream, string_view text) -> IOResult;
} // namespace up
