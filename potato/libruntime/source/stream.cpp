// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "stream.h"

#include "potato/spud/string.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/vector.h"

auto up::readBinary(Stream& stream, vector<up::byte>& out) -> IOResult {
    if (!stream.canRead() || !stream.canSeek()) {
        return IOResult::UnsupportedOperation;
    }

    auto const size = stream.remaining();
    auto const offset = out.size();
    out.resize(offset + size);

    auto read = out.subspan(offset);
    return stream.read(read);
}

auto up::readBinary(Stream& stream) -> IOReturn<vector<up::byte>> {
    vector<up::byte> data;
    auto rs = readBinary(stream, data);
    return {rs, std::move(data)};
}

auto up::readText(Stream& stream, string& out) -> IOResult {
    if (!stream.canRead() || !stream.canSeek()) {
        return IOResult::UnsupportedOperation;
    }

    auto const size = stream.remaining();
    string_writer writer;

    auto const uncommitted = writer.acquire(size);
    auto bytes = uncommitted.as_bytes();

    auto const rs = stream.read(bytes);

    writer.commit(uncommitted.first(bytes.size()));

    out = std::move(writer).to_string();

    return rs;
}

auto up::readText(Stream& stream) -> IOReturn<string> {
    string text;
    auto rs = readText(stream, text);
    return {rs, std::move(text)};
}

auto up::writeAllText(Stream& stream, string_view text) -> IOResult {
    if (!stream.canWrite()) {
        return IOResult::UnsupportedOperation;
    }

    auto const textSpan = span(text.data(), text.size());
    return stream.write(textSpan.as_bytes());
}
