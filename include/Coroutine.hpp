#pragma once

#include "Board.hpp"

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>

class PlayerTask {
public:
    struct promise_type {
        std::optional<Move> current_move;
        std::exception_ptr exception;

        PlayerTask get_return_object() {
            return PlayerTask{
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(Move move) noexcept {
            current_move = move;
            return {};
        }

        void return_void() noexcept {}

        void unhandled_exception() noexcept {
            exception = std::current_exception();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    PlayerTask() noexcept = default;
    explicit PlayerTask(handle_type handle) noexcept : handle_(handle) {}

    PlayerTask(const PlayerTask&) = delete;
    PlayerTask& operator=(const PlayerTask&) = delete;

    PlayerTask(PlayerTask&& other) noexcept
        : handle_(std::exchange(other.handle_, nullptr)) {}

    PlayerTask& operator=(PlayerTask&& other) noexcept {
        if (this != &other) {
            destroy();
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }

    ~PlayerTask() { destroy(); }

    bool valid() const noexcept { return handle_ != nullptr; }

    bool done() const noexcept { return !handle_ || handle_.done(); }

    bool resume() {
        if (!handle_ || handle_.done()) {
            return false;
        }
        handle_.promise().current_move.reset();
        handle_.resume();

        if (handle_.promise().exception) {
            std::rethrow_exception(handle_.promise().exception);
        }
        return !handle_.done() && handle_.promise().current_move.has_value();
    }

    std::optional<Move> last_move() const noexcept {
        if (!handle_) {
            return std::nullopt;
        }
        return handle_.promise().current_move;
    }

private:
    void destroy() noexcept {
        if (handle_) {
            handle_.destroy();
            handle_ = nullptr;
        }
    }

    handle_type handle_{nullptr};
};
