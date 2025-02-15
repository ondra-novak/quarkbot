#pragma once

#include "fastdecimal.h"
#include <chrono>

namespace quarkbot
{

using Price = FastDecimal;
using Quantity = FastDecimal;

class Side {
public:
    enum _ {
        UNDEFINED,
        BID,
        ASK,

    };
    constexpr int get_sign() const {
        return _val == BID?1:_val == ASK?-1:0;
    }
    constexpr std::string_view to_string() const {
        return _val == BID?"BID":_val == ASK?"ASK":"UNDEFINED";
    }
    constexpr _ value() const {return _val;}
    constexpr Side(_ val):_val(val) {}
    constexpr Side():_val(UNDEFINED) {}
    constexpr bool operator==(const Side &other) const = default;
protected:
    _ _val;
};


using TimeStamp = std::chrono::system_clock::time_point;
using Duration = std::chrono::system_clock::duration;
using TimerID = std::size_t;


class OrderSource {
public:
    enum _ {
        UNKNOWN,        ///< uknown source
        STRATEGY,       ///< order has been created by strategy
        RESTORED,       ///< order has been restored from the database
        EXTERNAL,       ///< order has been created outside of strategy
    };

    _ value() const {return _val;}

    constexpr std::string_view to_string() const {
        switch(_val) {
            case STRATEGY: return "STRATEGY";
            case RESTORED: return "RESTORED";
            case EXTERNAL: return "EXTERNAL";
            default: return "UNKNOWN";
        }
    }
    constexpr OrderSource(_ val):_val(val) {}
    constexpr OrderSource():_val(UNKNOWN) {}
    constexpr bool operator==(const OrderSource &other) const = default;
protected:
    _ _val;
};

class MarketType {
public:
    enum _ {
        normal,     ///< normal market type
        inversed,   ///< inversed market type
    };

    _ value() const {return _val;}

    constexpr std::string_view to_string() const {
        switch(_val) {
            case normal: return "normal";
            case inversed: return "inversed";
            default: return "unknown";
        }
    }
    constexpr MarketType(_ val):_val(val) {}
    constexpr MarketType():_val(normal) {}
    constexpr bool operator==(const MarketType &other) const = default;

    double calculate_pnl(Price open_price, Price close_price, double total_quantity) {
        if (_val == normal) {
            return (close_price - open_price) * total_quantity;
        } else {
            return (1.0/open_price - 1.0/close_price) * total_quantity;
        }
    }

    double to_normal_price(Price price) const {
        if (_val == normal) return price;
        return 1.0/price;
    }
    Quantity to_normal_quantity(Quantity q) const {
        if (_val == normal) return q;
        return -q;
    }
    double calc_volume(double price, double quantity) const {
        if (_val == normal) return price * quantity;
        else return quantity;
    }

    double calc_quantity_from_volume(double price, double volume) const {
        if (_val == normal) return volume / price;
        else return volume;
    }

protected:
    _ _val;
};


template<typename Target>
class IClonable {
public:
    using Interface = Target;

    virtual Target *clone() const = 0;
    virtual Target *clone(void *address) const = 0;
    virtual size_t get_object_size() const  = 0;
    constexpr virtual ~IClonable() = default;
};

template<typename Base>
class Clonable: public Base {
public:
    using Base::Base;

    virtual Clonable *clone() const override {return new Clonable(*this);}
    virtual Clonable *clone(void *address) const override  {return new(address) Clonable(*this);}
    virtual size_t get_object_size() const override  {return sizeof(Clonable);}
    constexpr virtual ~Clonable() = default;
};


struct KeyValue {
        std::string_view key;
        std::string_view value;
};


} // namespace quarkbot
