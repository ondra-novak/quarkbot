#pragma once

#include "../interface/instrument.h"
#include "../interface/event_target.h"

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace quarkbot {


template<std::invocable<Instrument, MarketEvents> Fn>
class Subscriber {
public:


    Subscriber(Fn fn):_fn(fn) {}
    ~Subscriber() {
        clear();
    }

    ///Sets subscription
    /**
     * @param instrument instrument
     * @param events subscribed events (to unsubscribe, set to empty)
     * @param target target where events has been pushed
     */
    void set_subscription(const Instrument &instrument,
               MarketEvents events, IEventTarget *target) {

        std::unique_lock _(_mx);
        std::vector<Target> &v = _reg[instrument];
        MarketEvents all_events = {};
        bool unsub = !events.any();
        auto iter = v.begin();
        auto end = v.end();
        bool found = false;
        while (iter != end) {
            Target &t = *iter;
            if (t.t == target) {
                found = true;
                if (unsub) {
                    if (&t != &v.back()) {
                        std::swap(t, v.back());
                        v.pop_back();
                        --end;
                        continue;
                    }
                } else {
                    t.events = events;
                }
            }
            all_events |= t.events;
            ++iter;
        }
        if (!found && !unsub) {
            v.push_back({target, events});
            all_events |= events;
        }
        if (v.empty()) {
            _reg.erase(instrument);
        }

        _fn(instrument, all_events);
    }

    ///broadcast instrument's market event
    /**
     * @param instrument instrument
     * @param events event/events
     * @retval true broadcasted
     * @retval false no registration
     */
    bool broadcast(const Instrument &instrument, MarketEvents events) {
        std::shared_lock _(_mx);
        auto iter = _reg.find(instrument);
        if (iter == _reg.end()) return false;
        for (const Target &t: iter->second) {
            if ((t.events & events).any()) {
                t.t->push_event(Event(MarketEventOnInstrument{instrument, events}));
            }
        }
        return true;
    }

    ///clear everything
    void clear() {
        std::unique_lock lk(_mx);
        auto tmp = std::move(_reg);
        lk.unlock();
        for (auto &[instr, vec]: tmp) {
            for (Target &t: vec) {
                t.t->push_event(Event(MarketEventOnInstrument{instr, MarketEvent::end_of_stream}));
            }
        }
    }


protected:
    Fn _fn;


    struct Target {
        IEventTarget *t;
        MarketEvents events;
    };

    std::shared_mutex _mx;
    std::unordered_map<Instrument, std::vector<Target>, Instrument::Hasher > _reg;



};

}
