// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

// detect platform
#if defined(_WIN32) || defined(_WINDOWS)
#    if !defined(_WIN64)
//#        error "Only Win64 is supported on Windows"
#    endif
#    define UP_PLATFORM_WINDOWS 1
#    define UP_PLATFORM_WIN64 1
#    define UP_PLATFORM_WIN32 1
#elif __APPLE__
#    define UP_PLATFORM_APPLE 1
#    define UP_PLATFORM_POSIX 1
#    include "TargetConditionals.h"
#    if TARGET_OS_IPHONE
#        define UP_PLATFORM_IOS 1
#    else
#        define UP_PLATFORM_MACOS 1
#    endif
#elif __linux
#    define UP_PLATFORM_LINUX 1
#    define UP_PLATFORM_POSIX 1
#    if defined(__ANDROID__)
#        define UP_PLATFORM_ANDROID 1
#    endif
#else
#    error "Unsupported platform"
#endif

// detect build configurations
#if defined(_DEBUG) || !defined(NDEBUG)
#    define UP_BUILD_DEBUG 1
#else
#    define UP_BUILD_RELEASE 1
#endif

// detect architecture and details
#if defined(_M_X64)
#    define UP_ARCH_LITTLE_ENDIAN 1
#    define UP_ARCH_INTEL 1
#    define UP_ARCH_64 1
#    define UP_ARCH_LLP64 1
#    define UP_ARCH_CACHELINE 64
#elif defined(__x86_64__)
#    define UP_ARCH_LITTLE_ENDIAN 1
#    define UP_ARCH_INTEL 1
#    define UP_ARCH_64 1
#    define UP_ARCH_LP64 1
#    define UP_ARCH_CACHELINE 64
#else
//#    error "Unsupported architecture"
#endif

// ensure C++17
#if __cplusplus < 201703L
#    error "C++17 or higher is required"
#endif

// compiler detection
#if defined(_MSC_VER)
#    if _MSC_VER >= 1920
#        define UP_COMPILER_MICROSOFT 1
#    else
#        error "Unsupported Visual C++ compiler version (requires 19.20 or higher from Visual Studio 2019)"
#    endif
#elif defined(__clang__)
#    define UP_COMPILER_CLANG 1
#elif defined(__GNUC__)
#    define UP_COMPILER_GCC 1
#else
#    error "Unsupported compiler"
#endif

// setup calling conventions, intrinsics, etc.
#if UP_PLATFORM_WINDOWS
#    define UP_STDCALL __stdcall
#    define UP_VECTORCALL __vectorcall
#else
#    define UP_STDCALL
#    define UP_VECTORCALL
#endif

#if UP_COMPILER_MICROSOFT
#    define UP_NOINLINE __declspec(noinline)
#    define UP_NORETURN __declspec(noreturn)
#    define UP_FORCEINLINE __forceinline
#    define UP_LIKELY(x) (x)
#    define UP_UNLIKELY(x) (x)
#    define UP_ASSUME(x) __assume((x))
#elif UP_COMPILER_CLANG || UP_COMPILER_GCC
#    define UP_NOINLINE [[gnu::noinline]]
#    define UP_NORETURN [[gnu::noreturn]]
#    define UP_FORCEINLINE [[gnu::always_inline]]
#    define UP_LIKELY(x) __builtin_expect((x), 1)
#    define UP_UNLIKELY(x) __builtin_expect((x), 0)
#    define UP_ASSUME(x) __builtin_assume((x))
#endif
