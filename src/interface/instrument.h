#pragma once

#include <memory>
#include "wrapper.h"
#include "account.h"
#include "instrument_info.h"
#include "position.h"
#include "market_event.h"
#include "tickdata.h"
#include "orderbook.h"

namespace quarkbot {

class IEventTarget;

struct Trade {
    Price price;
    Quantity volume;
    TimeStamp timestamp;
};

struct Index {
    Price price;
    TimeStamp timestamp;
};

struct Funding {
    double rate;
    TimeStamp expiration;
    TimeStamp timestamp;
};


class IInstrument {
public:
    virtual ~IInstrument() = default;

    virtual Account get_account() const = 0;
    virtual InstrumentInfo get_info() const = 0;
    virtual Positions get_positions() const = 0;
    virtual std::optional<TickData> get_ticker() const = 0;
    virtual std::optional<OrderBook> get_orderbook() const = 0;
    virtual std::optional<Trade> get_trade() const = 0;
    virtual std::optional<Index> get_index() const = 0;
    virtual std::optional<Funding> get_funding() const = 0;

    class Null;

};

class IInstrument::Null: public IInstrument {
public:
    virtual Account get_account() const override {return {};}
    virtual InstrumentInfo get_info() const override {return {};}
    virtual Positions get_positions() const override {return {};}

    virtual std::optional<TickData> get_ticker() const override {return {};}
    virtual std::optional<OrderBook> get_orderbook() const override {return {};}
    virtual std::optional<Trade> get_trade() const override {return {};}
    virtual std::optional<Index> get_index() const override {return {};}
    virtual std::optional<Funding> get_funding() const override {return {};}
};

///Information about instrument
/**
 * Informations returned from the instrument are cached
 * You need to explicity request update of the instrument
 * to receive current instrument state
 *
 * The instrument object is always associated with an account
 */
class Instrument: public Wrapper<IInstrument> {
public:
    using Wrapper<IInstrument>::Wrapper;

    ///Retrieve associated account
    Account get_account() const {return _ptr->get_account();}
    ///Retrieve account info
    InstrumentInfo get_info() const {return _ptr->get_info();}
    ///retrieves opened positions
    Positions get_positions() const {return _ptr->get_positions();}

    std::optional<TickData> get_ticker() const {
        return _ptr->get_ticker();
    }
    std::optional<OrderBook> get_orderbook() const {
        return _ptr->get_orderbook();
    }
    std::optional<Trade> get_trade() const {
        return _ptr->get_trade();
    }
    std::optional<Index> get_index() const {
        return _ptr->get_index();
    }
    std::optional<Funding> get_funding() const {
        return _ptr->get_funding();
    }

};



}
