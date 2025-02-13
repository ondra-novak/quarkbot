#pragma once

#include <variant>
#include "instrument.h"
#include "flagmap.h"
#include "market_event.h"

namespace quarkbot {




class Event {
public:

    struct MarketEvent {
        Instrument instr;
        FlagMap<MarketEvent> events;
        
    };


    using variant_def = std::variant<
        std::monostate
        //add events here
    >;

    template<typename ... Args>
    requires(std::is_constructible_v<variant_def, Args...>)
    Event(Args && ... args):_ev_data(std::forward<Args>(args)...) {}




protected:

    variant_def _ev_data;
};


}