#pragma once

#include "../interface/coroutines.h"
#include "../lib/minicoro/coro_scheduler.h"

namespace quarkbot {

class IScheduler {
public:
    virtual ~IScheduler() = default;
    virtual awaitable<void> sleep_until_alertable(alert_flag_type &alert_flag, std::chrono::system_clock::time_point tp) = 0;
    virtual void alert(alert_flag_type &alert_flag) = 0;
    virtual std::chrono::system_clock::time_point get_current_time() const = 0;
};

}
