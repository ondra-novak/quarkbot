#pragma once

#include "basic_types.h"
#include <optional>
#include <vector>

namespace quarkbot {

DECLARE_ENUM_CLASS(PositionDirection,
        undefined,      ///<position is not defined
        flat,           ///<position is flat
        buy_long,       ///<position is buy long
        sell_short      ///<position is sell short
);

template<typename X>
requires(std::is_arithmetic_v<X>)
X operator * (PositionDirection s, X val) {return (s == PositionDirection::sell_short?-1:s==PositionDirection::buy_long?1:0) * val;}
template<typename X>
requires(std::is_arithmetic_v<X>)
X operator * (X val, PositionDirection s) {return val * (s == PositionDirection::sell_short?-1:s==PositionDirection::buy_long?1:0);}

struct Position {
    PositionDirection type;      ///< position type (long/short)
    Quantity quantity;      ///< amount of position
    std::optional<TimeStamp> open_time;    ///< time when position has been opened
    std::optional<Price> open_price;       ///< open price (can be aggregated)
    std::optional<Quantity> initial_margin;        ///< amount of money blocked for this position
    std::optional<Quantity> maintenance_margin;    ///< amount of money blocked for maintenance margin
};

using Positions = std::vector<Position>;


}
