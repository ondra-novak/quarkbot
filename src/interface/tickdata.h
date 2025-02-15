#pragma once


#include "basic_types.h"

namespace quarkbot {


class TickData {
public:

    TimeStamp tp = {};  //snapshot time

    Price bid = {};  //current bid price
    Price ask = {};  //current ask price
    Price last = {};    //last execution price
    Price index = {};   //index price (if known) otherwise zero
    Quantity bid_volume = {}; //current volume on bid
    Quantity ask_volume = {}; //current volume on ask
    Quantity cum_volume = {};  //cumulative volume
    unsigned long cum_trades = 0; //total trades

    friend std::ostream &operator << (std::ostream &s, const TickData &tk) {
        s << tk.bid << '(' << tk.bid_volume << ") <-- " << tk.last << '(' << tk.cum_volume << ')' << " --> " << tk.ask << '(' << tk.ask_volume << ')';
        return s;
    }

    ///retrieve count trades happened between two market events
    friend unsigned long count_trades(const TickData &prev, const TickData &cur) {
        return cur.cum_trades > prev.cum_trades?cur.cum_trades - prev.cum_trades:cur.cum_trades;
    }

    ///retrieve total volume happened betwenn two market events
    friend double volume(const TickData &prev, const TickData &cur) {
        return cur.cum_volume > prev.cum_volume?cur.cum_volume - prev.cum_volume:cur.cum_volume;
    }

};


}
