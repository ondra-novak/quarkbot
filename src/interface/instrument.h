#pragma once

#include <memory>
#include "wrapper.h"
#include "account.h"
#include "instrument_info.h"
#include "position.h"


namespace quarkbot {



class IInstrument {
public:
    virtual ~IInstrument() = default;

    virtual Account get_account() const = 0;
    virtual InstrumentInfo get_info() const = 0;
    virtual Positions get_positions() const = 0;


    class Null;

};

class IInstrument::Null: public IInstrument {
public:
    virtual Account get_account() const override {return {};}  
    virtual InstrumentInfo get_info() const override {return {};}
    virtual Positions get_positions() const override {return {};}
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

};



}