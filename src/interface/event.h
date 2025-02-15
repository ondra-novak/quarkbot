#pragma once

#include "coroutines.h"
#include <variant>
#include "instrument.h"
#include "market_event.h"
#include "order.h"
#include "fill.h"



namespace quarkbot {



struct MarketEventOnInstrument {
    Instrument instr;
    MarketEvents events;
};


struct ExternalMessage {
    std::string channel;
    std::string message;
    unsigned int conversation_id;
};

//notify that order has been created and is ready to be placed
//the context must handle this operation (as a batch)
struct EventOrderPlace {
    Order order;
};

//notify that order has been cancelled
//the context must handle this operation (as a batch
struct EventOrderCancel {
    Order order;
};

class Event {
public:


    struct QuitEvent {};

    using variant_def = std::variant<
        std::monostate,
        prepared_coro, //any async call finished here
        MarketEventOnInstrument,
        Order,
        OrderUpdate,
        ExternalMessage,
        QuitEvent,
        std::exception_ptr,
        EventOrderPlace,
        EventOrderCancel
    >;

    template<typename ... Args>
    requires(std::is_constructible_v<variant_def, Args...>)
    Event(Args && ... args):_ev_data(std::forward<Args>(args)...) {}

    bool is_market_event() const {return std::holds_alternative<MarketEventOnInstrument>(_ev_data);}
    bool is_order_event() const {return std::holds_alternative<Order>(_ev_data);}
    bool is_external_event() const {return std::holds_alternative<ExternalMessage>(_ev_data);}
    bool is_exception() const {return std::holds_alternative<std::exception_ptr>(_ev_data);}

    const MarketEventOnInstrument &get_market_event() const {return std::get<MarketEventOnInstrument>(_ev_data);}
    const Order &get_order_event() const {return std::get<Order>(_ev_data);}
    const ExternalMessage &get_external_event() const {return std::get<ExternalMessage>(_ev_data);}
    const std::exception_ptr &get_exception() const {return std::get<std::exception_ptr>(_ev_data);}

    variant_def &get_underlying_data() {return _ev_data;}
    const variant_def &get_underlying_data() const {return _ev_data;}
protected:

    variant_def _ev_data;
};


}
