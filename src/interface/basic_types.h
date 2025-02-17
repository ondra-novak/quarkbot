#pragma once

#include "fastdecimal.h"
#include "../lib/utils/named_enum_class.h"
#include <chrono>

namespace quarkbot
{

using Price = FastDecimal;
using Quantity = FastDecimal;

DECLARE_ENUM_CLASS(Side,
            undefined, ///<undefined side
            bid,       ///<at bid side
            ask);      ///<at ask side

template<typename X>
requires(std::is_arithmetic_v<X>)
X operator * (Side s, X val) {return (s == Side::ask?-1:s==Side::bid?1:0) * val;}
template<typename X>
requires(std::is_arithmetic_v<X>)
X operator * (X val, Side s) {return val * (s == Side::ask?-1:s==Side::bid?1:0);}

using TimeStamp = std::chrono::system_clock::time_point;
using Duration = std::chrono::system_clock::duration;
using TimerID = std::size_t;

DECLARE_ENUM_CLASS(OrderSource,
        unknown,        ///< Unknown source
        strategy,       ///< order has been created by strategy
        restored,       ///< order has been restored from the database
        external       ///< order has been created outside of strategy
);

DECLARE_ENUM_CLASS(MarketTypeBase,
        normal,     ///< normal market type
        inversed    ///< inversed market type
);

class MarketType: public MarketTypeBase {
public:
    using MarketTypeBase::MarketTypeBase;
    constexpr double calculate_pnl(Price open_price, Price close_price, double total_quantity) {
        if (_value == normal) {
            return (close_price - open_price) * total_quantity;
        } else {
            return (1.0/open_price - 1.0/close_price) * total_quantity;
        }
    }

    constexpr double to_normal_price(Price price) const {
        if (_value == normal) return price;
        return 1.0/price;
    }
    constexpr Quantity to_normal_quantity(Quantity q) const {
        if (_value == normal) return q;
        return -q;
    }
    constexpr double calc_volume(double price, double quantity) const {
        if (_value == normal) return price * quantity;
        else return quantity;
    }

    constexpr double calc_quantity_from_volume(double price, double volume) const {
        if (_value == normal) return volume / price;
        else return volume;
    }
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

class ConfigError: public std::exception {
public:
    ConfigError(std::string field_name):_field_name(field_name) {}
    virtual const char *what() const noexcept override {
        std::ostringstream b;
        b << "Error in configuration - field: " << _field_name;
        _buffer = std::move(b).str();
        return _buffer.c_str();
    }
protected:
    std::string _field_name;
    mutable std::string _buffer;

};


} // namespace quarkbot
