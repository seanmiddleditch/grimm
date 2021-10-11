// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "concepts.h"
#include "traits.h"

namespace up {
    template <typename Signature>
    class delegate_ref;

    template <typename T>
    delegate_ref(T) -> delegate_ref<signature_t<T>>;

    namespace _detail {
        template <typename F, typename R, typename... P>
        R delegate_ref_thunk(void const* obj, P&&... params) {
            F const& f = *static_cast<F const*>(obj);
            if constexpr (std::is_void_v<R>) {
                f(std::forward<P>(params)...);
            }
            else {
                return f(std::forward<P>(params)...);
            }
        };

        template <typename ReturnType, typename... ParamTypes>
        struct delegate_ref_holder {
            using call_t = ReturnType (*)(void const*, ParamTypes&&...);

            delegate_ref_holder() = delete;
            delegate_ref_holder(delegate_ref_holder const&) = default;
            delegate_ref_holder& operator=(delegate_ref_holder const&) = default;

            template <callable_r<ReturnType, ParamTypes...> Functor>
            // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
            delegate_ref_holder(Functor&& functor) noexcept requires(!same_as<Functor, delegate_ref_holder>) {
                using FunctorType = std::remove_reference_t<Functor>;
                _call = &_detail::delegate_ref_thunk<FunctorType, ReturnType, ParamTypes...>;
                _functor = &functor;
            }

            call_t _call = nullptr;
            void const* _functor = nullptr;
        };
    } // namespace _detail
} // namespace up

template <typename ReturnType, typename... ParamTypes>
class up::delegate_ref<ReturnType(ParamTypes...)> {
private:
    using holder_t = _detail::delegate_ref_holder<ReturnType, ParamTypes...>;

public:
    delegate_ref() = delete;
    delegate_ref(delegate_ref const&) noexcept = default;
    delegate_ref& operator=(delegate_ref const&) noexcept = default;

    template <callable_r<ReturnType, ParamTypes...> Functor>
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    /*implicit*/ delegate_ref(Functor&& functor) noexcept requires(!same_as<Functor, delegate_ref>)
        : _holder(std::forward<Functor>(functor)) {}

    template <callable_r<ReturnType, ParamTypes...> Functor>
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    delegate_ref& operator=(Functor&& functor) noexcept requires(!same_as<Functor, delegate_ref>) {
        _holder = holder_t(std::forward<Functor>(functor));
        return *this;
    }

    // template <typename... InParamTypes>
    ReturnType operator()(ParamTypes... params) const {
        return _holder._call(_holder._functor, std::forward<ParamTypes>(params)...);
    }

private:
    template <typename Functor>
    void assign(Functor const& functor);

    holder_t _holder;
};
