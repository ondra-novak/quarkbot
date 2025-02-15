#include "context.h"
#include "../interface/exchange.h"
#include "../interface/mq.h"
#include "prepared_order.h"

namespace quarkbot {

StrategyContextImpl::StrategyContextImpl(IScheduler &sch,
        IMessageQueue &mq,
        Database db, std::string_view db_prefix, Instruments instruments)
:_sch(sch)
,_mq(mq)
,_db(std::move(db))
,_batch(_db.new_batch())
,_db_prefix(db_prefix)
,_db_prefix_len(db_prefix.size())
,_instruments(std::move(instruments))
{
}


thread_local StrategyContextImpl *StrategyContextImpl::current_strategy = nullptr;
void StrategyContextImpl::handle_coro_exception() {
    if (current_strategy) current_strategy->handle_exception();
}

void StrategyContextImpl::init_exception_handler() {
    current_strategy = this;
    async_unhandled_exception = &handle_coro_exception;
}



void StrategyContextImpl::push_event(Event event) {
    bool send_alert = false;
    auto &data = event.get_underlying_data();
    if (std::holds_alternative<EventOrderPlace>(data)) {
        EventOrderPlace &ev = std::get<EventOrderPlace>(data);
        _batch_place.push_back(std::move(ev.order));
    } else if (std::holds_alternative<EventOrderCancel>(data)) {
        EventOrderCancel &ev = std::get<EventOrderCancel>(data);
        _batch_cancel.push_back(std::move(ev.order));
    } else {
         std::lock_guard _(_mx);
         send_alert = _queue.empty();
        _queue.push_back(std::move(event));
    }
    if (send_alert) {
        _sch.alert(_alert);
    }
}



coroutine<void> StrategyContextImpl::main_loop() {

    std::unique_lock lk(_mx);
    while(true) {
        init_exception_handler();
        _event_time = _sch.get_current_time();

        Event ev;
        while (!_queue.empty()) {
            while (pop_event(ev)) {
                if (std::visit([this](auto &val){
                        return process_event(val);
                    }, ev.get_underlying_data())) {
                    _event_dist.broadcast(ev);
                }
                flush_batches();
            }
            if (_quit_flag) co_return;
            auto tp = _internal_scheduler.get_first_scheduled_time();
            while (tp && *tp < _event_time) {
                auto r = _internal_scheduler.remove_first();
                r(false);
                flush_batches();
                tp = _internal_scheduler.get_first_scheduled_time();
            }
            if (_queue.empty()) {
                _idle_dist.broadcast();
                flush_batches();
            }
        }
        if (_idle_dist.empty()) {
            auto fst = _internal_scheduler.get_first_scheduled_time();
            co_await _sch.sleep_until_alertable(_alert, fst?*fst:TimeStamp::max());
        }

    }

}

awaitable<const Event&> StrategyContextImpl::on_event() {
    return _event_dist();
}

awaitable<void> StrategyContextImpl::on_idle() {
    return _idle_dist();
}

awaitable<void> StrategyContextImpl::on_exception() {
    return _exception_dist();
}

bool StrategyContextImpl::pop_event(Event &event) {
    std::lock_guard _(_mx);
    if (_queue.empty()) return false;
    event = std::move(_queue.front());
    _queue.pop_back();
    return true;
}

void StrategyContextImpl::handle_exception() {
    auto exp = std::current_exception();
    if (exp) {
        push_event(exp);
    }
}


bool StrategyContextImpl::process_event(prepared_coro &ev) {
    ev();   //just run now
    return false;
}

bool StrategyContextImpl::process_event(MarketEventOnInstrument &ev) {
    auto iter = _me_per_instrument.find(ev.instr);
    if (iter != _me_per_instrument.end()) {
        iter->second.broadcast(ev.events);
        if (iter->second.empty()) _me_per_instrument.erase(iter);
    }
    _me_dist.broadcast(ev);
    return true;
}

bool StrategyContextImpl::process_event(Order &ev) {
    //Store fills
    for (const Fill &fill: ev.get_fills())  {
        _db_prefix.append(fill_prefix);
        Database::append_number(
                static_cast<std::uint64_t>(
                        std::chrono::duration_cast<std::chrono::microseconds>(fill.time.time_since_epoch()).count()),
                std::back_inserter(_db_prefix));
        _db_prefix.append(fill.fill_id);
        _batch.set_key(_db_prefix, std::string_view(reinterpret_cast<const char *>(&fill),sizeof(fill)));
        _db_prefix.resize(_db_prefix_len);
    }

    //Store order status
    Instrument i = ev.get_instrument();
    auto ser = dynamic_cast<const IInstrumentOrderSerialization *>(i.get_handle().get());
    if (ser) { //serialization must be supported
        auto sr = ser->serialize_order(ev);
        _db_prefix.append(order_prefix).append(sr.first);
        if (ev.is_done()) {
            _batch.erase_key(_db_prefix);   //erase order if done
        } else {
            _batch.set_key(_db_prefix, sr.second);  //store its state
        }
        _db_prefix.resize(_db_prefix_len);
    }

    //distribute
    auto iter = _dist_per_order.find(ev);
    if (iter != _dist_per_order.end()) {
        iter->second.broadcast();
        if (iter->second.empty()) _dist_per_order.erase(iter);
    }
    //distribute
    _order_dist.broadcast(ev);
    return true;
}

bool StrategyContextImpl::process_event(OrderUpdate &ev) {
    Order ord = ev->apply_update();
    if (process_event(ord)) {
        _event_dist.broadcast(Event(ord));
    }
    return false;

}

bool StrategyContextImpl::process_event(ExternalMessage &ev) {
    _mq_dist.broadcast(ev);
    return true;
}

bool StrategyContextImpl::process_event(Event::QuitEvent &) {
    _quit_flag = true;
    return false;
}

awaitable<Order> StrategyContextImpl::on_order_update() {
    return _order_dist();
}

awaitable<void> StrategyContextImpl::on_order_update(Order o) {
    if (o.is_done()) return nullptr;
    return _dist_per_order[o]();
}

awaitable<const MarketEventOnInstrument &> StrategyContextImpl::on_market() {
    return _me_dist();
}

awaitable<const ExternalMessage&> StrategyContextImpl::on_external_message() {
    return _mq_dist();
}

awaitable<FlagMap<MarketEvent> > StrategyContextImpl::on_market(Instrument instrument) {
    return _me_per_instrument[instrument]();
}

awaitable<bool> StrategyContextImpl::sleep_until(TimeStamp at) {
    return [this, at](awaitable<bool>::result r) {
        _internal_scheduler.schedule_at(std::move(r), at, 0);
    };
}

awaitable<bool> StrategyContextImpl::sleep_until(TimeStamp at, TimerID &id) {
    return [this, at, &id](awaitable<bool>::result r) {
        id = _next_timer_id++;
        _internal_scheduler.schedule_at(std::move(r), at, id);
    };
}

awaitable<bool> StrategyContextImpl::sleep_for(Duration at) {
    return sleep_until(_sch.get_current_time()+at);
}

awaitable<bool> StrategyContextImpl::sleep_for(Duration at, TimerID &id) {
    return sleep_until(_sch.get_current_time()+at, id);
}

bool StrategyContextImpl::interrupt(TimerID id) {
    auto r = _internal_scheduler.remove_by_ident(id);
    auto p = r(true);
    bool ret = static_cast<bool>(p);
    _queue.push_back(Event(std::move(p)));
    return ret;

}

TimeStamp StrategyContextImpl::get_event_time() const {
    return _event_time;
}


bool StrategyContextImpl::process_event(std::exception_ptr &ev) {
    try {
        std::rethrow_exception(ev);
    } catch (...) {
        _exception_dist.broadcast();
    }
    return false;
}

void StrategyContextImpl::set_subscription(const Instrument &instrument, MarketEvents events) {
    Account acc = instrument.get_account();
    auto exchange = acc.get_handle()->get_exchange();
    exchange->set_subscription(instrument, events, this);
}

void StrategyContextImpl::subscribe_channel(const std::string_view channel) {
    _mq.subscribe(this, channel);
}

void StrategyContextImpl::unsubscribe_channel(const std::string_view channel) {
    _mq.unsubscribe(this, channel);
}

bool StrategyContextImpl::send_message(std::string_view channel,
        std::string_view message, unsigned int conversation_id) {
    return _mq.send_message(this, channel, message, conversation_id);
}


awaitable<void> StrategyContextImpl::update_account(const Account &account) {
    return account.get_handle()->get_exchange()->update_account(this, account);
}

void StrategyContextImpl::cancel_all_orders(const Instrument &instr) {
    return instr.get_account().get_handle()->get_exchange()->cancel_all_orders(this, instr);
}

Order StrategyContextImpl::place_order(const Instrument &instrument,
        Quantity quantity, const OrderSetup &params, std::string_view label) {
    Order ord = instrument.get_account().get_handle()->get_exchange()
                ->create_order(this, instrument, quantity, params, label);
    _batch_place.push_back(ord);
    return ord;
}

Order StrategyContextImpl::prepare_order(const Instrument &instrument, std::string_view label) {
    return Order(std::make_shared<PreparedOrder>(instrument, label, this));
}

awaitable<void> StrategyContextImpl::update_instrument(
        const Instrument &instrument, MarketEvents events) {
    return instrument.get_account().get_handle()
            ->get_exchange()->update_instrument(this, instrument, events);
}


void StrategyContextImpl::flush_batches() {
    _batch_cancel.clear();
    _batch_place.clear();
    _db.commit_batch(_batch);
}

void StrategyContextImpl::var_set_string(std::string_view name, std::string_view value) {
    _db_prefix.append(var_prefix).append(name);
    _batch.set_key(_db_prefix, value);
    _db_prefix.resize(_db_prefix_len);

}


std::optional<std::string> StrategyContextImpl::var_get_string(
        std::string_view name) {
    _db_prefix.append(var_prefix).append(name);
    auto r = _db.get_value(_db_prefix);
    _db_prefix.erase(_db_prefix_len);
    return r;
}

async_generator<KeyValue> StrategyContextImpl::var_list_range(
        std::string_view from_range, std::string_view to_range,
        unsigned int skip_prefix) {
    std::string from = _db_prefix;
    std::string to = _db_prefix;
    from.append(from_range);
    to.append(to_range);
    return _db.range(from, to, _db_prefix_len + skip_prefix);
}

void StrategyContextImpl::var_erase(std::string_view key) {
    _db_prefix.append(var_prefix).append(key);
    auto r = _db.get_value(_db_prefix);
    _db_prefix.erase(_db_prefix_len);
}

static async_generator<Fill> extract_fill_from_kv(async_generator<KeyValue> gen) {
    auto awt = gen();
    while (co_await awt.has_value()) {
        const KeyValue &kv = awt.await_resume();
        const Fill *f = reinterpret_cast<const Fill *>(kv.value.data());
        co_yield std::move(*f);
        awt = gen();
    }
}

async_generator<Fill> StrategyContextImpl::get_fills_from(TimeStamp tp) {
    std::string from = _db_prefix;
    from.append(fill_prefix);
    std::string to = from;
    Database::append_number(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            TimeStamp::max().time_since_epoch()).count()), std::back_inserter(to));
    Database::append_number(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            tp.time_since_epoch()).count()), std::back_inserter(from));
    return extract_fill_from_kv(_db.range(from, to, 0));

}

async_generator<Fill> StrategyContextImpl::get_recent_fills() {
    std::string from = _db_prefix;
    from.append(fill_prefix);
    std::string to = from;
    Database::append_number(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            TimeStamp::max().time_since_epoch()).count()), std::back_inserter(from));
    Database::append_number(static_cast<std::uint64_t>(0), std::back_inserter(to));
    return extract_fill_from_kv(_db.range(from, to, 0));
}

const StrategyContextImpl::Instruments &StrategyContextImpl::get_instruments() const {
    return _instruments;
}

void StrategyContextImpl::unsubscribe_all() {
    for (const auto &x: _instruments) {
        x.instrument.get_account().get_handle()
                ->get_exchange()->set_subscription(x.instrument, MarketEvents{},this);
    }
    _mq.unsubscribe_all(this);
}


async_generator<Order> StrategyContextImpl::restore_orders(async_generator<KeyValue> gen, const Instrument &instr) {
    auto awt = gen();
    auto ex = instr.get_account().get_handle()->get_exchange();
    while (co_await awt.has_value()) {
        const KeyValue &kv = awt.await_resume();
        if (kv.key.substr(0, order_prefix.length()) != order_prefix) break;
        std::string_view key = kv.key.substr(order_prefix.length());
        auto awt2 =  ex->restore_order(this, instr, key, kv.value);
        if (co_await awt2.has_value()) {
            co_yield awt2.await_resume();
        }
    }
}


async_generator<Order> StrategyContextImpl::restore_open_orders(const Instrument &instr) {

    std::string beg = _db_prefix;
    beg.append(order_prefix);
    auto iterator = _db.iterate(beg,false,_db_prefix_len);
    return restore_orders(std::move(iterator), instr);



}



}
