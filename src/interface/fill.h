#pragma once

#include <memory>
#include "wrapper.h"


namespace quarkbot {

class IFill {
public:
    virtual ~IFill() = default;


    class Null;

};

class IFill::Null: public IFill {
public:
};

class Fill: public Wrapper<IFill> {
public:
    using Wrapper<IFill>::Wrapper;

};



}