#pragma once

#include <memory>
#include "wrapper.h"
#include "account.h"


namespace quarkbot {

class IInstrumentInfo {
public:
    virtual ~IInstrumentInfo() = default;


    class Null;

};

class IInstrumentInfo::Null: public IInstrumentInfo {
public:
};

class InstrumentInfo: public Wrapper<IInstrumentInfo> {
public:
    using Wrapper<IInstrumentInfo>::Wrapper;

};



}