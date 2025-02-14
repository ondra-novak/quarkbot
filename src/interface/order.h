#pragma once

#include <memory>
#include "wrapper.h"
#include "basic_types.h"
#include "account.h"
#include "order_setup.h"
#include "fill.h"
#include <span>

namespace quarkbot
{

class Order;

class OrderState {
public:
    enum _ {
        unknown,        ///< state of an order is unknown
        associated,     ///< order is associated with account and instrument
        pending_new,    ///< order has been posted to exchange, no status received yet
        open,           ///< order has been received by exchange and has been activated
        filled,         ///< order has been fully filled
        cancelled,      ///< order has been cancelled
        rejected,       ///< order has been rejected
        expired,        ///< order expired (canceled)
        pending_cancel, ///< cancel request has been sent
        pending_replace,///< replace request has been sent (sent to order being replaced)
        replaced,       ///< order has been replaced (sent to replaced - final state)
        failed,         ///< order failed (error message is available)
        restored        ///< order has been restored from the database and no state is known yet
    };
    constexpr _ value() const {return _val;}

    constexpr std::string_view to_string() const {
        switch (_val) {
            case pending_new: return "new";     
            case associated: return "associated";
            case open: return "open";
            case filled: return "filled";
            case cancelled: return "canceled";
            case rejected: return "rejected";
            case expired: return "expired";            
            case pending_cancel: return "pending_cancel";            
            case pending_replace: return "pending_replace";            
            case replaced: return "replaced";            
            case failed: return "failed";
            case restored: return "restored";
            default: return "unknown";
        }
    }
    constexpr OrderState(_ val):_val(val) {}
    constexpr OrderState():_val(unknown) {}
    constexpr bool operator==(const OrderState &other) const = default;
    constexpr bool is_final() const {
        return _val != pending_new && _val != open && _val != pending_cancel && _val != pending_replace;
    }
protected:
    _ _val;
};
        



class IOrder {
public:
    using State = OrderState;

    virtual ~IOrder() = default;
    virtual Account get_account() const = 0;
    virtual Instrument get_instrument() const = 0;
    virtual OrderState get_state() const  = 0;
    virtual const OrderSetup &get_setup() const = 0;
    virtual Quantity get_total_quantity() const = 0;
    virtual Quantity get_filled_quantity() const = 0;
    virtual Price get_avg_fill_price() const = 0;
    virtual std::span<Fill> get_fills() const = 0;
    virtual std::string get_error() const = 0;
    virtual void cancel() const = 0;
    virtual Order replace(Quantity new_quantity, const OrderSetup &params, std::string_view label = {}) const = 0;
    class Null;
};

class IOrder::Null: public IOrder {
public:
    static constexpr OrderType::no_setup no_setup_order = {};

    virtual Account get_account() const override {return {};};
    virtual Instrument get_instrument() const override {return {};}
    virtual OrderState get_state() const  override {return {};}
    virtual const OrderSetup &get_setup() const {return no_setup_order;}
    virtual Quantity get_total_quantity() const {return {};}
    virtual Quantity get_filled_quantity() const {return {};}
    virtual Price get_avg_fill_price() const {return {};}
    virtual std::span<Fill> get_fills() const {return {};}
    virtual Order replace(Quantity new_quantity, const OrderSetup &params, std::string_view label = {}) const;
    virtual std::string get_error() const {return {};}
    virtual void cancel() const {}

    
};

class Order: public Wrapper<IOrder> {
public:
    using State = OrderState;

    using Wrapper<IOrder>::Wrapper;
    /// get associated account
    Account get_account() const {
        return _ptr->get_account();
    }
    /// get associated instrument
    Instrument get_instrument() const {
        return _ptr->get_instrument();
    }
    /// get order state
    /** The state of the order is updated before the order is returned to the strategy,
     * You cannot pool state (unless doing it in on_idle())
     */
    OrderState get_state() const {
        return _ptr->get_state();
    }
    ///Get order's original setup
    const OrderSetup &get_setup() const {
        return _ptr->get_setup();
    }
    ///Get order's total quantity to fill
    Quantity get_total_quantity() const {
        return _ptr->get_total_quantity();
    }
    ///Get current order's filled quantity
    Quantity get_filled_quantity() const {
        return _ptr->get_filled_quantity();
    }
    ///Get average fill price
    Price get_avg_fill_price() const {
        return _ptr->get_avg_fill_price();
    }
    ///Retrieve recent fills
    /**
     * The fills are carried with current event.
     * Each event generates new list of fills.     
     */
    std::span<Fill> get_fills() const {
        return _ptr->get_fills();
    }
    ///Cancel the order
    /** 
     * The order may receive pending_cancel and it is scheduled for cancelation
     * @note The function doesn't modify current state until next update
     */
    void cancel() const {
        return _ptr->cancel();
    }
    ///Replace the order
    /**
     * @param new_quantity new quantity
     * @param params new order setup (should be same order type)
     * @param label optional label
     * @return new order
     * @exception OrderError If exception is thrown, original order is not affected
     * 
     * @note you can replace done orders, they are treat as associated
     */
     Order replace(Quantity new_quantity, const OrderSetup &params, std::string_view label = {}) const {
        return _ptr->replace(new_quantity,params, label);
    }
    ///returns true if order is done
    bool is_done() const {
        return _ptr->get_state().is_final();        
    }
    ///returns error message - applied for orders rejected and failed
    std::string get_error() const {
        return _ptr->get_error();
    }

};

///contains update of the order which must be applied on the order
///when the event is passed to the strategy

class IOrderState {
public:
    virtual ~IOrderState() = default;
    ///apply update and return associated order 
    virtual Order apply_update() = 0;
};

using OrderUpdate = std::shared_ptr<IOrderState>;

class OrderError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

inline Order IOrder::Null::replace(Quantity , const OrderSetup &, std::string_view ) const {return Order();}


///Interface should be inherited by an instrument to support serialization of orders into the database
class IInstrumentOrderSerialization : public IInstrument {
public:
    ///Serialize order to string representation which can be stored in the database 
    /**
     * @param order which has been associated with current instrument
     * @return binary representation
     * 
     * @note the binary representation doesn't need to store whole order if the order is 
     * is also available from exchange report. It can only contain order's ID
     * 
     * The string must contain some identification of the exchange and instrument, as
     * it must be able to reject binary representation for order which is not
     * created for this instrument. During restoration phase all stored orders
     * are processed by all instruments.
     * 
     * To restore order, use restore_order
     */
    virtual std::string serialize_order(const Order &order) const = 0;    
    ///Restores order from binary representation
    /**
     * @param order_bin binary representation
     * @return Order object. If the binary representation is not recognized, returned
     * object is not valid order. Otherwise the order is created in "restored". 
     */
    virtual Order restore_order(const std::string_view order_bin) const = 0;

};


} // namespace quarkbot
