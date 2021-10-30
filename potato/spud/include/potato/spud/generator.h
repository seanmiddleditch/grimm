// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_assertion.h"

#include <type_traits>
#include <version>

#if defined(__cpp_lib_coroutine) && __has_include(<coroutine>)
#    include <coroutine>
namespace coro_std = std;
#elif __has_include(<experimental/coroutine>)
#    include <experimental/coroutine>
namespace coro_std = std::experimental;
#else
#    error "coroutines support is required"
#endif

namespace up {
    template <typename ItemT>
    class generator;

    namespace _detail {
        template <typename T>
        class generator_promise {
        public:
            using value_type = std::remove_reference_t<T>;
            using reference = std::conditional_t<std::is_reference_v<T>, T, T&>;
            using pointer = value_type*;

            generator_promise() noexcept = default;

            generator<T> get_return_object() noexcept;

            void return_void() const noexcept {}
            coro_std::suspend_always initial_suspend() const noexcept { return {}; }
            coro_std::suspend_always final_suspend() const noexcept { return {}; }

            coro_std::suspend_always yield_value(reference value) noexcept {
                _value = &value;
                return {};
            }

            reference value() const noexcept { return *_value; }

            void unhandled_exception() const noexcept { UP_SPUD_ASSERT(false, "unhandled exception in generator"); }
            template <typename U>
            coro_std::suspend_never await_transform(U&& value) = delete;

            void rethrow_if_exception() const noexcept {}

        private:
            pointer _value = nullptr;
        };

        struct generator_sentinel {};

        template <typename T>
        class generator_iterator {
            using handle_type = coro_std::coroutine_handle<_detail::generator_promise<T>>;

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = typename generator_promise<T>::value_type;
            using reference = typename generator_promise<T>::reference;
            using pointer = typename generator_promise<T>::pointer;

            generator_iterator() noexcept = default;
            explicit generator_iterator(handle_type handle) noexcept : _coro(handle) {}

            friend bool operator==(generator_iterator const& lhs, generator_sentinel) noexcept {
                return lhs._coro == nullptr || lhs._coro.done();
            }

            generator_iterator& operator++() {
                _coro.resume();
                return *this;
            }

            reference operator*() const noexcept { return _coro.promise().value(); }

        private:
            handle_type _coro = nullptr;
        };
    } // namespace _detail

    template <class ItemT>
    class [[nodiscard]] generator {
        using handle_type = coro_std::coroutine_handle<_detail::generator_promise<ItemT>>;

    public:
        using promise_type = _detail::generator_promise<ItemT>;
        using iterator = _detail::generator_iterator<ItemT>;
        using sentinel = _detail::generator_sentinel;

        generator() noexcept = default;
        ~generator() { reset(); }

        generator(generator const&) = delete;
        generator(generator&& rhs) noexcept : _coro(rhs._coro) { rhs._coro = nullptr; }

        generator& operator=(generator const&) = delete;
        generator& operator=(generator&& rhs) noexcept {
            if (this != &rhs) {
                reset();
                _coro = rhs._coro;
                rhs._coro = nullptr;
            }
            return *this;
        }

        iterator begin() noexcept;
        sentinel end() const noexcept { return {}; }

        explicit operator bool() const noexcept { return _coro != nullptr && !_coro.done(); }

        void reset() noexcept {
            if (_coro != nullptr) {
                _coro.destroy();
                _coro = nullptr;
            }
        }

    private:
        explicit generator(handle_type handle) noexcept : _coro(handle) {}

        handle_type _coro;

        friend class _detail::generator_promise<ItemT>;
    };

    template <typename T>
    auto _detail::generator_promise<T>::get_return_object() noexcept -> generator<T> {
        return generator{coro_std::coroutine_handle<generator_promise>::from_promise(*this)};
    }

    template <typename ItemT>
    auto generator<ItemT>::begin() noexcept -> generator::iterator {
        if (_coro != nullptr) {
            _coro.resume();
        }
        return iterator{_coro};
    }

} // namespace up
