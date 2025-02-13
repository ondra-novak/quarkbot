#pragma once

#include "fastdecimal.h"

namespace quarkbot
{

using Price = FastDecimal;
using Quantity = FastDecimal;

class Side {
public:
    enum _ {
        undefined,
        bid,
        ask,

    };
    int get_sign() const {
        return _val == bid?1:_val == ask?-1:0;
    }
    std::string_view to_string() const {
        return _val == bid?"BID":_val == ask?"ASK":"UNDEFINED";
    }
    constexpr Side(_ val):_val(val) {}
    constexpr Side():_val(undefined) {}
    constexpr bool operator==(const Side &other) const=default;
protected:
    _ _val;
};


} // namespace quarkbot
