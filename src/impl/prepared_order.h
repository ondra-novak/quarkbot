#pragma once

#include "../interface/order.h"
#include "../interface/instrument.h"

namespace quarkbot {


class PreparedOrder: public IOrder::Null {
public:
    PreparedOrder(const Instrument &instrument, std::string_view label, IEventTarget *target)
        :_instr(instrument)
        ,_label(label)
        ,_event_target(target) {}
    PreparedOrder(const PreparedOrder &) = delete;
    PreparedOrder &operator=(const PreparedOrder &) = delete;

    virtual Instrument get_instrument() const override {
        return _instr;
    }
    virtual IEventTarget *get_event_target() const override {
        return _event_target;
    }
    virtual OrderState get_state() const  override {
        return OrderState::associated;
    }
    virtual Order replace(Quantity new_quantity, const OrderSetup &params, std::string_view label) const override {
        Order ord = _instr.get_account().get_handle()->get_exchange()->create_order(
                _event_target, _instr, new_quantity, params, label.empty()?_label:label);
        _event_target->push_event(EventOrderPlace(ord));
        return ord;
    }

protected:
    Instrument _instr;
    std::string _label;
    IEventTarget *_event_target;
};


}


