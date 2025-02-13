#pragma once

#include "coroutines.h"
#include "event.h"

namespace quarkbot {


class StrategyContext {
public:

    virtual ~StrategyContext() = default;

    virtual awaitable<Event> wait_event() = 0;



};



}