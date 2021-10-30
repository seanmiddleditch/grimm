// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/thread_util.h"

#if !defined(_GNU_SOURCE)
#    define _GNU_SOURCE // for glibc
#endif

#include <pthread.h>

void up::setCurrentThreadName(zstring_view name) noexcept {
    pthread_setname_np(pthread_self(), name.c_str());
}
