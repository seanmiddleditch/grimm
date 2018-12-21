// Copyright (C) 22015 Sean Middleditch, all rights reserverd.

#include "callstack.h"
#include "platform.h"
#include "string.h"

#include <execinfo.h>

int gm::CallStackReader::readCallstack(array_view<uintptr> addresses, int skip) {
    void* buffer;

    int max = std::max<int>(0, addresses.size() - skip);
    int count = backtrace(&buffer, max);
    skip = std::min(skip, count);

    std::memcpy(addresses.data(), static_cast<uintptr*>(buffer) + skip, std::min(count, max));

    return count - skip;
}

bool gm::CallStackReader::tryResolveCallstack(array_view<uintptr const> addresses, array_view<CallStackRecord> out_records) {
#if !defined(NDEBUG)
    void* const addrs = const_cast<uintptr*>(addresses.data());
    char** symbols = backtrace_symbols(&addrs, addresses.size());

    for (auto index = 0; index != addresses.size(); ++index) {
        CallStackRecord& record = out_records[index];
        record.symbol = string_view(symbols[index]);
    }

    free(symbols);

    return true;
#else
    return false;
#endif // !defined(NDEBUG)
}
