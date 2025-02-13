#pragma once

#include <variant>
#include "instrument.h"
#include "flagmap.h"
#include "market_event.h"
#include "order.h"
#include "fill.h"

namespace quarkbot {
class Event {
public:

    struct MarketEvent {
        Instrument instr;
        FlagMap<MarketEvent> events;
    };
    struct ExternalMessage {
        std::string channel;
        std::string message;
        unsigned int conversation_id;
    };

    using variant_def = std::variant<
        std::monostate,
        prepared_coro, //any async call finished here
        MarketEvent,
        Order,
        OrderUpdate,
        ExternalMessage
    >;

    template<typename ... Args>
    requires(std::is_constructible_v<variant_def, Args...>)
    Event(Args && ... args):_ev_data(std::forward<Args>(args)...) {}

    bool is_market_event() const {return std::holds_alternative<MarketEvent>(_ev_data);}
    bool is_order_event() const {return std::holds_alternative<Order>(_ev_data);}
    bool is_external_event() const {return std::holds_alternative<ExternalMessage>(_ev_data);}

    const MarketEvent &get_market_event() const {return std::get<MarketEvent>(_ev_data);}

    variant_def &get_underlying_data() {return _ev_data;}
    const variant_def &get_underlying_data() const {return _ev_data;}



protected:

    variant_def _ev_data;
};


}