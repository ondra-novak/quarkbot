#pragma once

#include <memory>
#include "wrapper.h"
#include "account.h"
#include "instrument_info.h"


namespace quarkbot {

class IInstrument {
public:
    virtual ~IInstrument() = default;

    virtual Account get_account() const = 0;
    virtual InstrumentInfo get_info() const = 0;

    class Null;

};

class IInstrument::Null: public IInstrument {
public:
    virtual Account get_account() const override {return {};}  
    virtual InstrumentInfo get_info() const override {return {};}
};

class Instrument: public Wrapper<IInstrument> {
public:
    using Wrapper<IInstrument>::Wrapper;

    Account get_account() const {return _ptr->get_account();}
    InstrumentInfo get_info() const {return _ptr->get_info();}
};



}