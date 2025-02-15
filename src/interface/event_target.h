#pragma once

#include "event.h"

namespace quarkbot {

class IEventTarget {
public:

    virtual ~IEventTarget() = default;
    virtual void push_event(Event event) = 0;
};



}
