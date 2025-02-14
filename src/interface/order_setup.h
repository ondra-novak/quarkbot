#pragma once
#include <typeinfo>
#include "account.h"

namespace quarkbot {

struct OrderOptions {

};


class OrderSetup: public IClonable<OrderSetup> {
public:

    template<typename T>
    constexpr bool is() const {
        return typeid(*this) == typeid(T);
    }

    template<typename T>
    constexpr const T &as() const {
        return static_cast<const T &>(*this);
    }

    virtual std::string to_string() const {return "no-order-setup";}
};



#define ORDER_SETUP_AUTO_CLONE \
    virtual OrderSetup *clone() const override {return new auto(*this);} \
    virtual OrderSetup *clone(void *addr) const override {return new(addr) auto(*this);} \
    virtual std::size_t object_size() const override {return sizeof(*this);} 

class OrderSetupWithOptions : public OrderSetup{
public:
    using Options = OrderOptions;
    constexpr OrderSetupWithOptions(Options options):_options(std::move(options)) {}
    const OrderOptions &get_options() const {return _options;}
protected:
    Options _options;
};

class OrderSetup_Market: public OrderSetupWithOptions {
    public:
        constexpr OrderSetup_Market(Side side,  Options options = {}) 
            :OrderSetupWithOptions(std::move(options))
            ,_side(side) {}
        Side get_side() const {return _side;}
        virtual std::string to_string() const override {
            return std::format("{} MARKET",_side.to_string());
        }
    protected:
        Side _side;
        Quantity _quantity;
    };
    
    class OrderSetup_Limit: public OrderSetup_Market {
    public:
        constexpr OrderSetup_Limit(Side side, Price limit_price, Options options = {}) 
            :OrderSetup_Market(side,std::move(options))
            ,_limit_price(limit_price) {}
        const Price get_limit_price() const {return _limit_price;}
        virtual std::string to_string() const override {
            return std::format("{} LIMIT {}",_side.to_string(), static_cast<std::string>(_limit_price));
        }
    protected:
        Price _limit_price;
    };
    
    class OrderSetup_StopLimit: public OrderSetup_Limit {
    public:
        constexpr OrderSetup_StopLimit(Side side,  Price stop_price, Price limit_price, Options options = {}) 
            :OrderSetup_Limit(side, limit_price,std::move(options))
            ,_stop_price(stop_price) {}
        const Price get_stop_price() const {return _stop_price;}
        virtual std::string to_string() const override {
            return std::format("{} STOP {} LIMIT {}",_side.to_string(), 
                    static_cast<std::string>(_stop_price), 
                    static_cast<std::string>(_limit_price));
        }
    protected:
        Price _stop_price;
    };
    
    class OrderSetup_Stop: public OrderSetup_Market {
        public:
            constexpr OrderSetup_Stop(Side side,  Price stop_price, Options options = {}) 
                :OrderSetup_Market(side,std::move(options))
                ,_stop_price(stop_price) {}
            const Price get_limit_price() const {return _stop_price;}
            virtual std::string to_string() const override {
                return std::format("{} STOP {}",_side.to_string(), 
                    static_cast<std::string>(_stop_price));
            }
        protected:
            Price _stop_price;
    };
    
    class OrderSetup_Transfer: public OrderSetup {
    public:
        OrderSetup_Transfer(const Account &to_account) 
            :_to_account(to_account) {}
        const Account &get_account() const {return _to_account;}        
        virtual std::string to_string() const override {
            return "TRANSFER";
        }
protected:
        Account _to_account;
    };

namespace OrderType {
    using no_setup = Clonable<OrderSetup>;
    using market = Clonable<OrderSetup_Market>;
    using limit = Clonable<OrderSetup_Limit>;
    using stop_limit = Clonable<OrderSetup_StopLimit>;
    using stop = Clonable<OrderSetup_Stop>;
    using transfer = Clonable<OrderSetup_Transfer>;
}



}