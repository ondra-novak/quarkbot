#pragma once

#include <memory>
#include "wrapper.h"


namespace quarkbot {

class IInstrument {
public:
    virtual ~IInstrument() = default;


    class Null;

};

class IInstrument::Null: public IInstrument {
public:
    
};

class Instrument: public Wrapper<IInstrument> {
public:
    using Wrapper<IInstrument>::Wrapper;

};



}