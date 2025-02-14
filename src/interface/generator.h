#pragma once
#include <coroutine>
#include <exception>
#include <optional>

template<typename T>
class generator {
public:
    struct promise_type {
        std::optional<T> current_value;
        std::exception_ptr exception;

        auto get_return_object() {
            return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    explicit generator(handle_type h) : coro(h) {}
    generator(const generator&) = delete;
    generator(generator&& other) noexcept : coro(other.coro) {
        other.coro = nullptr;
    }

    ~generator() {
        if (coro) coro.destroy();
    }

    bool next() {
        if (!coro.done()) coro.resume();
        return !coro.done();
    }

    T value() const {
        return *coro.promise().current_value;
    }

private:
    handle_type coro;
};




#endif /* SRC_INTERFACE_GENERATOR_H_ */
