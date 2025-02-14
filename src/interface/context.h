#pragma once

#include "coroutines.h"
#include "event.h"
#include "order_setup.h"
#include "iterator.h"
#include <functional>

namespace quarkbot {


class StrategyContext {
public:

    struct LabeledInstrument {
        std::string label;
        Instrument instrument;
    };

    using Instruments = std::vector<LabeledInstrument>;

    virtual ~StrategyContext() = default;

    ///Retrieve list of configured instruments
    /**
     * @return list of configured instruments. Each instrument has label, so it
     * is easy to find out purpose of each instrument
     */
    virtual const Instruments &get_instruments() const = 0;

    ///Finds instrument by its label
    /** Convenient function to find instrument by its label. It expects that instrument
     * has an unique label. Note that sequential search is performed, so it
     * should be called during startup and store found instrument into a variable.
     *
     * @param label label
     * @return found instrument, returns undefined instrument if not found
     */
    Instrument find_instrument_by_label(std::string_view label) const {
        const Instruments &lst = get_instruments();
        auto iter = std::find_if(lst.begin(), lst.end(), [&](const LabeledInstrument &l){
            return l.label == label;
        });
        if (iter != lst.end()) return iter->instrument;
        else return Instrument();
    }

    ///wait for any event
    virtual awaitable<const Event &> on_event() = 0;
    ///wait for order update event
    virtual awaitable<Order> on_order_update() = 0;
    ///wait for order update event for specific order
    virtual awaitable<void> on_order_update(Order o) = 0;
    ///wait for any market event
    virtual awaitable<const MarketEventOnInstrument&> on_market()  = 0;
    ///wait for any market event
    virtual awaitable<const ExternalMessage &> on_external_message()  = 0;
    ///wait for any market event on specific instrument
    virtual awaitable<MarketEvents> on_market(Instrument instrument)  = 0;
    ///sleep until time is reached
    virtual awaitable<bool> sleep_until(TimeStamp at) = 0;
    ///sleep until time si reached interruptable (fills TimerID with an id)
    virtual awaitable<bool> sleep_until(TimeStamp at, TimerID &id) = 0;
    ///sleep for given duration
    virtual awaitable<bool> sleep_for(Duration at) = 0;
    ///sleep for given duration interruptable (fills TimerID with an id)
    virtual awaitable<bool> sleep_for(Duration at, TimerID &id) = 0;
    ///interrupt given sleep identified by id
    virtual bool interrupt(TimerID id) = 0;
    ///wait for strategy idle
    virtual awaitable<void> on_idle() = 0;
    ///wait on exception - the awaiter doesn't return a value but right after co_await the exception is available as std::current_exception
    virtual awaitable<void> on_exception() = 0;
    ///returns current event's time
    virtual TimeStamp get_event_time() const = 0;

    /// Place order
    /**
     * @param instrument instrument where to put order
     * @param quantity amount of assets to trade
     * @param params order configuration (OrderType::xxxx)
     * @param label optional string label - all fills made this order will receive same label
     * @return Instance of new order.
     * @exception OrderError if order cannot be created, the exception is thrown. However
     * any error happened during order's processing is reported through OrderState
     */
    virtual Order place_order(const Instrument &instrument,
                              Quantity quantity,
                              const OrderSetup &params,
                              std::string_view label = {}) = 0;

    ///prepare order instance
    /**
     * The returned value is empty order instance which is in state "associated".
     * It is associated with an instrument and with context. You can
     * use replace() function to replace this order with an actual order later
     * without need to access its instrument and associated context
     *
     * This order doesn't receive updates, it is in done state already.
     */
    virtual Order prepare_order(const Instrument &instrument) = 0;

    ///subscribe for market events
    virtual void subscribe(const Instrument &instrument, MarketEvent event) = 0;
    ///unsubscribe market events
    virtual void unsubscribe(const Instrument &instrument, MarketEvent event) = 0;
    ///subscribe message channel
    virtual void subscribe_channel(const std::string_view channel) = 0;
    ///unsubscribe message channel
    virtual void unsubscribe_channel(const std::string_view channel) = 0;
    ///Store string to database
    virtual void var_set_string(std::string_view name, std::string_view value) = 0;

    ///retrieve string from database
    virtual std::optional<std::string> var_get_string(std::string_view name) = 0;


    ///Retrieve multiple variables from the database
    /**
     *  @param from_range starting range
     *  @param to_range ending range
     *  @return asynchronous generator you can iterate through the KeyValues
     */

    virtual async_generator<KeyValue> var_list_range(std::string_view from_range,
            std::string_view to_range, unsigned int skip_prefix = 0) = 0;

    virtual void var_erase(std::string_view key) = 0;
    ///Retrieve all recent fills
    /**
     * @return a generator which returns all fills in reverse order (so the very first
     * value is recent one)
     *
     * The generator eventually ends with very last fill, but you can destroy it anytime
     */
    virtual async_generator<Fill> get_recent_fills() = 0;

    ///Retrieve all fills from given timestamp (forward)
    /**
     * @param timestamp time when start
     * @return all fills from given timestamp
     */
    virtual async_generator<Fill> get_fills_from(TimeStamp tp) = 0;

    ///Restore open orders from database for given instrument
    /**Scans database and restores all orders for given instrument
     *
     * @param instr instrument
     * @return a generator which generates restored orders. All these orders are
     * subscribed for update and their update event arrives as very next event
     *
     * @note not restoring orders causes that database are filled by opened orders. The
     * opened order can be deleted only if it is checked for final state at the exchange
    */
    virtual async_generator<Order> restore_open_orders(const Instrument &instr) = 0;

    ///Cancel all orders on given instrument
    /** This function causes that all open orders are canceled. Note that
     * it cancels all order including orders outside of scope of this strategy.
     * All orders held by strategy should receive status canceled. They don't need
     * to receive status pending_cancel
     */
    virtual void cancel_all_orders(const Instrument &instr) = 0;

    ///Update accunt
    /**
     * All informations about account are cached. This function causes
     * that informations on the account are updated from the exchange
     *
     * @param account account to update
     * @return operation is asynchronous
     */
    virtual awaitable<void> update_account(const Account &account) = 0;

    ///Update instrument
    /**
     * All informations about account are cached. This function causes
     * that informations on the account are updated from the exchange.
     *
     * @param account account to update
     * @param events list of informations associated with given event. If no events are defined, just
     * basic informations are updated.
     * @return operation is asynchronous
     *
     * This function can be called when instrument is not subscribed. This allows to pool price or
     * order book in interval which should be longer than several seconds (as the pooling is
     * subject or rate limiting)
     *
     */
    virtual awaitable<void> update_instrument(const Instrument &instrument, FlagMap<MarketEvent> events) = 0;


    ///Send message to a MQ channel (zeromq)
    /**
     * @param channel channel name
     * @param message message content
     * @param conversation_id an identification of the conversation (optional)
     * @retval true message sent
     * @retval false no route to target (invalid channel?)
     */
    virtual bool send_message(std::string_view channel, std::string_view message, unsigned int conversation_id = 0) = 0;


};



}
