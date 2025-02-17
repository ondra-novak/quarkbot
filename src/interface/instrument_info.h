#pragma once

#include <memory>
#include "wrapper.h"
#include "account.h"
#include "fill.h"

namespace quarkbot {

DECLARE_ENUM_CLASS(AlignmentStrategy,
    defensive,  ///< align price or quantity by defensive strategy (less likely to fill)
    aggresive,  ///< align price or quantity by aggresive strategy (more likely to fill)
    nearest     ///< align price or quantity to nearest tick / lot size
);


class IInstrumentInfo {
public:
    virtual ~IInstrumentInfo() = default;

    virtual bool is_leveraged() const = 0;
    virtual bool can_short() const = 0;
    virtual Price get_tick_size() const = 0;
    virtual Quantity get_quantity_step() const = 0;
    virtual Quantity get_min_quantity_size() const = 0;
    virtual double get_min_volume() const = 0;
    virtual double get_fee_ratio() const = 0;
    virtual MarketType get_type() const = 0;
    virtual double get_quantity_multiplier() const = 0;
    virtual double get_price_multiplier() const = 0;

    class Null;

};

class IInstrumentInfo::Null: public IInstrumentInfo {
public:

    virtual bool is_leveraged() const override {return false;}
    virtual bool can_short() const override{return false;}
    virtual Price get_tick_size() const override {return {};}
    virtual Quantity get_quantity_step() const override {return {};}
    virtual Quantity get_min_quantity_size() const override {return {};}
    virtual double get_min_volume() const override {return {};}
    virtual double get_fee_ratio() const override {return {};}
    virtual MarketType get_type() const override {return {};}
    virtual double get_quantity_multiplier() const override {return 1.0;}
    virtual double get_price_multiplier() const override {return 1.0;}

};

class InstrumentInfo: public Wrapper<IInstrumentInfo> {
public:
    using Wrapper<IInstrumentInfo>::Wrapper;

    ///returns true if instrument is leveraged
    bool is_leveraged() const {
        return _ptr->is_leveraged();
    }
    ///returns true if instrument can be shorted
    bool can_short() const {
        return _ptr->can_short();
    }
    ///align price to specified tick
    /**
     * @param price price to align
     * @param alignemnt price aligment
     * @param side trading side
     * @param cover_fees shift price to cover potential fees.
     *   For example ASK side shifts price higher, because raised profit from sell covers fees.
     *   The same for BID side which shifts price lower
     * @return Price object which can be passed to the place_order
     *
     */
    Price align_price(double price, AlignmentStrategy aligment, Side side, bool cover_fees = false) const {
        Price tksz = _ptr->get_tick_size();
        double dtksz = tksz;
        if (side == Side::ask) {
            if (cover_fees) {
                price = price * (1.0 + get_fee_ratio());
            }
            switch (aligment.value()) {
                case AlignmentStrategy::aggresive:
                    return Price(std::floor(price/dtksz)*dtksz, tksz.get_decimal_count());
                case AlignmentStrategy::defensive:
                    return Price(std::ceil(price/dtksz)*dtksz, tksz.get_decimal_count());
                default:
                    return Price(std::round(price/dtksz)*dtksz, tksz.get_decimal_count());
            }
        } else if (side == Side::bid) {
            if (cover_fees) {
                price = price - (1.0 + get_fee_ratio());
            }
            switch (aligment.value()) {
                case AlignmentStrategy::aggresive:
                    return Price(std::ceil(price/dtksz)*dtksz, tksz.get_decimal_count());
                case AlignmentStrategy::defensive:
                    return Price(std::floor(price/dtksz)*dtksz, tksz.get_decimal_count());
                default:
                    return Price(std::round(price/dtksz)*dtksz, tksz.get_decimal_count());
            }
        } else {
            return Price(std::round(price/dtksz)*dtksz, tksz.get_decimal_count());
        }
    }
    ///align quantity to nearest contract size
    /**
     * @param quantity quantity to align
     * @param alignment startagy
     * @return Quantity object
     */
    Quantity align_quantity(double quantity, AlignmentStrategy aligment) const {
        Price lotsz = _ptr->get_quantity_step();
        double dlotsz = lotsz;
        switch (aligment.value()) {
            case AlignmentStrategy::aggresive:
                return Price(std::ceil(quantity/dlotsz)*dlotsz, lotsz.get_decimal_count());
            case AlignmentStrategy::defensive:
                return Price(std::floor(quantity/dlotsz)*dlotsz, lotsz.get_decimal_count());
            default:
                return Price(std::round(quantity/dlotsz)*dlotsz, lotsz.get_decimal_count());
        }
    }
    ///Get tick size
    Price get_tick_size() const {
        return _ptr->get_tick_size();
    }
    ///Get Quantity step
    Quantity get_quantity_step() const {
        return _ptr->get_quantity_step();
    }
    ///Get minimal tradable quantity size
    Quantity get_min_quantity_size() const {
        return _ptr->get_min_quantity_size();
    }
    ///Get minimal volume
    double get_min_volume() const {
        return _ptr->get_min_volume();
    }
    ///Get Fee ratio (if known). The value is always < 1
    double get_fee_ratio() const {
        return _ptr->get_fee_ratio();
    }
    ///Get market type
    MarketType get_type() const {
        return _ptr->get_type();
    }
    ///Get multiplier between quantity value and actual amount of assets
    /**
     * For example if multiplier is 100000, then quantity 0.1 means 10000 items.
     * This is tipical on Forex
     */
    double get_quantity_multiplier() const {
        return _ptr->get_quantity_multiplier();
    }
    ///Get price multiplier
    /**
     * Multiplier which converts price to real value. Can be used as FX rate, but if the rate doesn't change often
     * because it is static value
     */
    double get_price_multiplier() const {
        return _ptr->get_price_multiplier();
    }

    ///Generates MarketFillInfo structure which can be stored in database
    MarketFillInfo get_fill_info() const {
        return {
            get_type(),
            get_quantity_multiplier(),
            get_price_multiplier()
        };
    }

    ///Calculate minimal quantity at given price
    /** Because some exchanges also defines minimal volume, this function calculates absolute minimum
     * allowed quantity at given price
     */
    Quantity get_min_quantity_at_price(Price price) const {
        return  std::max(std::max(align_quantity(
            get_type().calc_quantity_from_volume(price * get_price_multiplier(), get_min_volume())/get_quantity_multiplier(),
            AlignmentStrategy::aggresive),get_min_quantity_size()), get_tick_size());
    }



};



}
