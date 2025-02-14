#pragma once

#include "basic_types.h"
#include "wrapper.h"
#include <vector>
#include <memory>

namespace quarkbot
{


class IAccount {
public:
    virtual ~IAccount() = default;

    virtual std::string get_currency() const  = 0;
    virtual int get_currency_decimal_paces() const  = 0;
    virtual double get_fx_ratio() const = 0;
    virtual Quantity get_tradable_balance() const = 0;
    virtual Quantity get_equity() const = 0;
    virtual Quantity get_blocked() const = 0;
    virtual bool is_leveraged() const = 0;    

    class Null;
};

class IAccount::Null: public IAccount {
public:

virtual std::string get_currency() const override {return "UNKNWON";};
virtual int get_currency_decimal_paces() const  override {return 0;}
virtual double get_fx_ratio() const override {return 1.0;}
virtual Quantity get_tradable_balance() const override {return {0,0};}
virtual Quantity get_equity() const override {return {0,0};}
virtual Quantity get_blocked() const override {return {0,0};}
virtual bool is_leveraged() const override {return false;}

};

///Information about account
/**
 * @note Informations held by this object are cached. You need to update
 * the account state by command StrategyContext::update_account() to
 * retrieve current state
 */
class Account: public Wrapper<IAccount> {
public:

    using Wrapper<IAccount>::Wrapper;

    ///Retrieve currency symbol
    /** Symbol can be used for printing, but should not be used for identification */
    std::string get_currency() const {
        return _ptr->get_currency();
    }
    ///Retrieves count of decimal places to be used to display currency value
    int get_currency_decimal_paces() const {
        return _ptr->get_currency_decimal_paces();
    }
    ///Retrieves FX ratio for currency associated with this account
    /**The actual value can be relative to anything, and its meaning
     * has only if it is used with ratio of other currency. To convert
     * value from account currency to (unknown) global currency, you
     * need to divide value by this rate. To convert global currency value
     * to this account currency value, you need to multiply value by
     * this rate
     * */
    double get_fx_ratio() const {
        return _ptr->get_fx_ratio();
    }
    /// Retrieve FX ratio between two accounts (if they have different currencies)
    /**
     *  @return a ratio. You need to multiply other value to receive value in currency
     * for this account
     */
    double get_fx_ratio(const Account &from) const {
        return get_fx_ratio()/from.get_fx_ratio();
    }
    ///Converts value to currency of this account
    /**
     * @param from reference to account from which currency to convert the value
     * @param value value to convert
     * @return value of this account currency
     */
    double fx_convert(const Account &from, double value) const {
        return get_fx_ratio(from) * value;
    }
    ///Retrieve tradable balance
    /**
     * This return amount of money available for opening new orders. For the spot 
     * accounts, this value is equal to amount of funds on the account minus
     * funds allocated for orders. For leverages accounts, this value
     * can contain total funds minus sum of funds used for orders and
     * position's margin
     */
    Quantity get_tradable_balance() const {
        return _ptr->get_tradable_balance();
    }

    ///Retrieves total equity
    /**
     * Contains funds + UPnL. For spot markets, its also evaluates value of all held assets on the account
     */
    Quantity get_equity() const {
        return _ptr->get_equity();
    }
    ///Retrieves amount of funds blocked in orders and positions
    Quantity get_blocked() const {
        return _ptr->get_blocked();
    }
    ///returns true, if account is leveraged (can trade leveraged instruments)
    bool is_leveraged() const {
        return _ptr->is_leveraged();
    }
        
};



} // namespace quarkbot
