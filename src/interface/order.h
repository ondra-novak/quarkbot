#pragma once

#include <memory>
#include "wrapper.h"
#include "basic_types.h"
#include "account.h"

namespace quarkbot
{

class IOrder {
public:
    virtual ~IOrder() = default;
    virtual Account get_account() const = 0;

    class Null;
};

class IOrder::Null: public IOrder {
public:
    virtual Account get_account() const override {return {};};

};

class Order: public Wrapper<IOrder> {
public:

    using Wrapper<IOrder>::Wrapper;
    Account get_account() const {
        return _ptr->get_account();
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


} // namespace quarkbot
