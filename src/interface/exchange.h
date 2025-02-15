#pragma once
#include "market_event.h"



namespace quarkbot {

class Instrument;
class IEventTarget;


class IExchange {
public:

    virtual ~IExchange() = default;

    virtual void set_subscription(
                    const Instrument &instrument,
                            MarketEvents events,
                            IEventTarget *target) = 0;

    virtual awaitable<void> update_account(IEventTarget *target, const Account acc) = 0;
    virtual awaitable<void> update_instrument(IEventTarget *target, const Instrument &instrument, MarketEvents events) = 0;
    virtual void cancel_all_orders(IEventTarget *target, const Instrument &instrument) = 0;
    virtual Order create_order(IEventTarget *target, const Instrument &instrument,
                    Quantity quantity, const OrderSetup &params, std::string_view label) = 0;

    ///Restore order stored in database
    /**
     * Creates order, registers event target to receive events, posts all fills, and posts order state
     * @param target target context
     * @param instrument instrument
     * @param key key retrieved from database
     * @param value value retrieved from database
     * @return order (async)
     */
    virtual awaitable<Order> restore_order(IEventTarget *target, const Instrument &instrument,
                    std::string_view key, std::string_view value) = 0;



};


}
