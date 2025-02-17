#pragma once

#include "ischeduler.h"
#include "../interface/context.h"
#include "../interface/event_target.h"
#include "../interface/database.h"
#include "../lib/minicoro/coro_distributor.h"

#include <mutex>
#include <queue>
#include <vector>



namespace quarkbot {

class IMessageQueue;
class IScheduler;

class StrategyContextImpl: public StrategyContext,
                           public IEventTarget
    {
public:


    StrategyContextImpl(IScheduler &sch,            //scheduler
                        IMessageQueue &msgq,        //message queue
                        Database db,                //database handler
                        std::string_view db_prefix, //database prefix
                        Instruments instruments);   //list of instruments

    virtual awaitable<const Event &> on_event() override;
    virtual awaitable<Order> on_order_update() override;
    virtual awaitable<void> on_order_update(Order o) override;
    virtual awaitable<const MarketEventOnInstrument &> on_market() override;
    virtual awaitable<const ExternalMessage &> on_external_message() override;
    virtual awaitable<MarketEvents> on_market(Instrument instrument)  override;
    virtual awaitable<bool> sleep_until(TimeStamp at)override;
    virtual awaitable<bool> sleep_until(TimeStamp at, TimerID &id) override;
    virtual awaitable<bool> sleep_for(Duration at)override;
    virtual awaitable<bool> sleep_for(Duration at, TimerID &id) override;
    virtual awaitable<void> on_idle() override;
    virtual awaitable<void> on_exception() override;
    virtual bool interrupt(TimerID id) override;
    virtual TimeStamp get_event_time() const override;

    virtual void var_set_string(std::string_view name, std::string_view value) override;
    virtual std::optional<std::string> var_get_string(std::string_view name) const override ;
    virtual async_generator<KeyValue> var_list_range(std::string_view from_range,
               std::string_view to_range, unsigned int skip_prefix) const override;
    virtual void var_erase(std::string_view key)override;
    virtual async_generator<Fill> get_recent_fills() const override;
    virtual async_generator<Fill> get_fills_from(TimeStamp tp) const override;
    virtual const Instruments &get_instruments() const override;

    virtual void push_event(Event event) override;
    virtual void set_subscription(const Instrument &instrument, MarketEvents event) override;

    virtual void subscribe_channel(const std::string_view channel) override;
    virtual void unsubscribe_channel(const std::string_view channel) override;
    virtual bool send_message(std::string_view channel, std::string_view message, unsigned int conversation_id = 0) override;


    void unsubscribe_all();
    virtual awaitable<void> update_account(const Account &account) override;
    virtual void cancel_all_orders(const Instrument &instr) override;
    virtual Order place_order(const Instrument &instrument, Quantity quantity,
            const OrderSetup &params, std::string_view label) override;
    virtual Order prepare_order(const Instrument &instrument, std::string_view label) override;
    virtual awaitable<void> update_instrument( const Instrument &instrument, MarketEvents events) override;
    virtual async_generator<Order> restore_open_orders(const Instrument &instr) override;

protected:
    using Queue = std::deque<Event>;
    using LocalScheduler = generic_scheduler<awaitable<bool>::result, TimeStamp, TimerID>;


    std::mutex _mx;
    IScheduler &_sch;
    IMessageQueue &_mq;
    alert_flag_type _alert;
    Queue _queue;
    bool _quit_flag = false;
    TimerID _next_timer_id = 1;
    TimeStamp _event_time;
    Database _db;
    Database::Batch _batch;
    std::string _db_prefix;
    std::size_t _db_prefix_len;
    Instruments _instruments;



    LocalScheduler _internal_scheduler;
    distributor<const Event &> _event_dist;
    distributor<void> _idle_dist;
    distributor<void> _exception_dist;
    distributor<const ExternalMessage &> _mq_dist;
    distributor<Order> _order_dist;
    distributor<const MarketEventOnInstrument &> _me_dist;
    std::unordered_map<Instrument, distributor<MarketEvents> , Instrument::Hasher>_me_per_instrument;
    std::unordered_map<Order, distributor<void> , Order::Hasher>_dist_per_order;
    std::vector<Order> _batch_place;
    std::vector<Order> _batch_cancel;

    static thread_local StrategyContextImpl *current_strategy;
    static constexpr std::string_view var_prefix = "#";
    static constexpr std::string_view order_prefix = "@";
    static constexpr std::string_view fill_prefix = "$";

    static void handle_coro_exception();

    coroutine<void> main_loop();
    bool pop_event(Event &event);


    void init_exception_handler();
    void handle_exception();

    template<typename X>
    bool process_event(X &) {return false;}
    bool process_event(prepared_coro &ev);
    bool process_event(MarketEventOnInstrument &ev);
    bool process_event(Order &ev);
    bool process_event(OrderUpdate &ev);
    bool process_event(ExternalMessage &ev);
    bool process_event(Event::QuitEvent &ev);
    bool process_event(std::exception_ptr &ev);

    void flush_batches();
    async_generator<Order> restore_orders(async_generator<KeyValue> gen, const Instrument &instr);
    };

}
