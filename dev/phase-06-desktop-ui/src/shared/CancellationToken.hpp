#pragma once

#include <atomic>

namespace shared {

class CancellationToken {
  public:
    void cancel() {
        cancelled_.store(true, std::memory_order_release);
    }

    [[nodiscard]] bool isCancelled() const {
        return cancelled_.load(std::memory_order_acquire);
    }

  private:
    std::atomic<bool> cancelled_{false};
};

} // namespace shared
