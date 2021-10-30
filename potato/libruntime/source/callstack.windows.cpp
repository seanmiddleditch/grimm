// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "callstack.h"

#include "potato/spud/int_types.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/platform.h"
#include "potato/spud/platform_windows.h"

#include <Tracy.hpp>

#pragma warning(disable : 4091)
#include <dbghelp.h>

namespace {

    class CallstackHelper {
    public:
        CallstackHelper();
        ~CallstackHelper();

        CallstackHelper(CallstackHelper const&) = delete;
        CallstackHelper& operator=(CallstackHelper const&) = delete;

        CallstackHelper(CallstackHelper&&) = delete;
        CallstackHelper& operator=(CallstackHelper&&) = delete;

        bool isInitialized() const { return _initialized; }
        up::uint captureStackTrace(up::uint skip, up::uint count, void** entries);
        void readSymbol(void* entry, PSYMBOL_INFO symInfo, PIMAGEHLP_LINE64 lineInfo);

        static CallstackHelper& instance();

    private:
        bool _initialized = false;
        HANDLE _process;

        using CaptureStackBackTraceType = USHORT(WINAPI*)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);

        HMODULE _kernelLib;
        CaptureStackBackTraceType _captureStackBackTrace;

#if !defined(NDEBUG)
        using SymInitializeType = BOOL(WINAPI*)(__in HANDLE, __in_opt PCTSTR, __in BOOL);
        using SymCleanupType = BOOL(WINAPI*)(__in HANDLE);
        using SymGetLineFromAddr64Type = BOOL(WINAPI*)(__in HANDLE, __in DWORD64, __out PDWORD, __out PIMAGEHLP_LINE64);
        using SymFromAddrType = BOOL(WINAPI*)(__in HANDLE, __in DWORD64, __out_opt PDWORD64, __inout PSYMBOL_INFO);

        HMODULE _dbghelpLib;
        SymInitializeType _symInitialize;
        SymCleanupType _symCleanup;
        SymGetLineFromAddr64Type _symGetLineFromAddr64;
        SymFromAddrType _symFromAddr;
        DWORD _initValue = ERROR_SUCCESS;
#endif // !defined(NDEBUG)
    };

    CallstackHelper::CallstackHelper() {
        _process = GetCurrentProcess();

        _kernelLib = LoadLibraryW(L"kernel32.dll");
        if (_kernelLib == nullptr) {
            return;
        }

        _captureStackBackTrace =
            reinterpret_cast<CaptureStackBackTraceType>(GetProcAddress(_kernelLib, "RtlCaptureStackBackTrace"));
        if (_captureStackBackTrace == nullptr) {
            return;
        }

#if !defined(NDEBUG)
        _dbghelpLib = LoadLibraryW(L"dbghelp.dll");
        if (_dbghelpLib == nullptr) {
            return;
        }

        _symInitialize = reinterpret_cast<SymInitializeType>(GetProcAddress(_dbghelpLib, "SymInitialize"));
        if (_symInitialize == nullptr) {
            return;
        }

        _symCleanup = reinterpret_cast<SymCleanupType>(GetProcAddress(_dbghelpLib, "SymCleanup"));
        if (_symCleanup == nullptr) {
            return;
        }

        _symGetLineFromAddr64 =
            reinterpret_cast<SymGetLineFromAddr64Type>(GetProcAddress(_dbghelpLib, "SymGetLineFromAddr64"));
        if (_symGetLineFromAddr64 == nullptr) {
            return;
        }

        _symFromAddr = reinterpret_cast<SymFromAddrType>(GetProcAddress(_dbghelpLib, "SymFromAddr"));
        if (_symFromAddr == nullptr) {
            return;
        }

#    if !defined(TRACY_ENABLE) // Tracy initializes the dbghelp.dll if enabled
        if (_symInitialize(_process, nullptr, TRUE) == FALSE) {
            _initValue = ::GetLastError();
            return;
        }
#    endif // !defined(TRACE_ENABL)
#endif // !defined(NDEBUG)

        _initialized = true;
    }

    CallstackHelper::~CallstackHelper() {
#if !defined(NDEBUG)
        if (_symCleanup != nullptr) {
            _symCleanup(_process);
        }

        _symCleanup = nullptr;
        _symInitialize = nullptr;
        _symGetLineFromAddr64 = nullptr;
        _symFromAddr = nullptr;

        FreeLibrary(_dbghelpLib);
#endif // !defined(NDEBUG)

        _captureStackBackTrace = nullptr;
        FreeLibrary(_kernelLib);
    }

    up::uint CallstackHelper::captureStackTrace(up::uint skip, up::uint count, void** entries) {
        if (_captureStackBackTrace != nullptr) {
            return _captureStackBackTrace(skip + 1, count, entries, nullptr);
        }

        return 0;
    }

#if !defined(NDEBUG)
    void CallstackHelper::readSymbol(void* entry, PSYMBOL_INFO symInfo, PIMAGEHLP_LINE64 lineInfo) {
        if (_symFromAddr != nullptr) {
            _symFromAddr(GetCurrentProcess(), DWORD64(entry), nullptr, symInfo);
        }

        DWORD displacement = 0;
        if (_symGetLineFromAddr64 != nullptr) {
            _symGetLineFromAddr64(GetCurrentProcess(), DWORD64(entry), &displacement, lineInfo);
        }
    }
#endif // !defined(NDEBUG)

    CallstackHelper& CallstackHelper::instance() {
        static CallstackHelper helper;
        return helper;
    }

} // anonymous namespace

auto up::callstack::readTrace(span<up::uintptr> addresses, up::uint skip) -> span<up::uintptr> {
    CallstackHelper& helper = CallstackHelper::instance();

    if (!helper.isInitialized()) {
        return {};
    }

    uint count = helper.captureStackTrace(
        skip + 1,
        static_cast<uint>(addresses.size()),
        reinterpret_cast<void**>(addresses.data()));

    return addresses.first(count);
}

auto up::callstack::resolveTraceRecords(span<up::uintptr const> addresses, span<TraceRecord> records)
    -> span<TraceRecord> {
#if !defined(NDEBUG)
    CallstackHelper& helper = CallstackHelper::instance();

    if (!helper.isInitialized()) {
        return {};
    }

    int const max = static_cast<int>(up::min(addresses.size(), records.size()));

    int constexpr kMaxNameLen = 128;

    std::aligned_union_t<sizeof(SYMBOL_INFO) + kMaxNameLen, SYMBOL_INFO> symbolStorage;
    auto symbolInfoPtr = reinterpret_cast<PSYMBOL_INFO>(&symbolStorage);
    IMAGEHLP_LINE64 imagehlpLine64;

    ZeroMemory(&symbolStorage, sizeof(symbolStorage));
    ZeroMemory(&imagehlpLine64, sizeof(imagehlpLine64));

    symbolInfoPtr->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbolInfoPtr->MaxNameLen = kMaxNameLen;

    for (auto index = 0; index != max; ++index) {
        helper.readSymbol(reinterpret_cast<void*>(addresses[index]), symbolInfoPtr, &imagehlpLine64);

        auto& record = records[index];
        record.address = addresses[index];
        record.symbol = string_view(static_cast<char*>(symbolInfoPtr->Name), symbolInfoPtr->NameLen);
        // record.filename = imagehlpLine64.FileName;
        record.line = imagehlpLine64.LineNumber;

        // show only the last directory and file name
        string_view filename(imagehlpLine64.FileName);
        auto pos = filename.find_last_of("/\\");
        if (pos != string_view::npos) {
            auto parentPos = filename.substr(0, pos).find_last_of("/\\");
            if (parentPos != string_view::npos) {
                pos = parentPos;
            }
            filename = filename.substr(pos + 1);
        }
        record.filename = filename;
    }

    return records.first(max);
#else
    return {};
#endif // !defined(NDEBUG)
}
