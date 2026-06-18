#pragma once

#include "Error.hpp"

#include <utility>

namespace shared {

template <typename T> struct Result {
    bool ok = false;
    T value{};
    AppError error{};

    static Result success(T value) {
        Result result;
        result.ok = true;
        result.value = std::move(value);
        return result;
    }

    static Result failure(AppError error) {
        Result result;
        result.ok = false;
        result.error = std::move(error);
        return result;
    }
};

template <> struct Result<void> {
    bool ok = false;
    AppError error{};

    static Result success() {
        Result result;
        result.ok = true;
        return result;
    }

    static Result failure(AppError error) {
        Result result;
        result.ok = false;
        result.error = std::move(error);
        return result;
    }
};

} // namespace shared
