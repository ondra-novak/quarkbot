#pragma once

#include <memory>
#include "wrapper.h"

namespace quarkbot
{

class IAccount {
public:
    virtual ~IAccount() = default;


    class Null;
};

class IAccount::Null: public IAccount {
public:
};

class Account: public Wrapper<IAccount> {
public:

    using Wrapper<IAccount>::Wrapper;

    
};



} // namespace quarkbot
