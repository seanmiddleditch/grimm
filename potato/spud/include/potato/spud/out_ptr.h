// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_assertion.h"

namespace up {
    template <typename S, typename P>
    class out_ptr_t;

    template <typename S>
    auto out_ptr(S& out) noexcept -> out_ptr_t<S, typename S::pointer>;
} // namespace up

template <typename S, typename P>
class up::out_ptr_t {
public:
    using pointer = P;

    out_ptr_t(S& out) noexcept : _out(out) {}
    ~out_ptr_t() noexcept(noexcept(_out.reset(_ptr))) { _out.reset(_ptr); }

    out_ptr_t(out_ptr_t const&) = delete;
    out_ptr_t& operator=(out_ptr_t&) = delete;

    operator pointer*() && noexcept {
        UP_ASSERT(_ptr == nullptr);
        return &_ptr;
    }

    // clang-format in VC is behaving differently than clang-format-10 on CI
    // clang-format off
    operator void**() noexcept {
        UP_ASSERT(_ptr == nullptr);
        return reinterpret_cast<void**>(&_ptr);
    }
    // clang-format on

private:
    pointer _ptr = nullptr;
    S& _out;
};

template <typename S>
auto up::out_ptr(S& out) noexcept -> out_ptr_t<S, typename S::pointer> {
    return out_ptr_t<S, typename S::pointer>(out);
}
