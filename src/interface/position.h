#pragma once

#include "basic_types.h"
#include <optional>
#include <vector>

namespace quarkbot {

class PositionType {
public:
    enum _ {
        UNDEFINED,
        LONG,
        SHORT,

    };
    int get_sign() const {
        return _val == LONG?1:_val == SHORT?-1:0;
    }
    std::string_view to_string() const {
        return _val == LONG?"LONG":_val == SHORT?"SHORT":"UNDEFINED";
    }
    constexpr PositionType(_ val):_val(val) {}
    constexpr PositionType():_val(UNDEFINED) {}
    constexpr bool operator==(const PositionType &other) const=default;
protected:
    _ _val;
};
        

struct Position {
    PositionType type;      ///< position type (long/short)
    Quantity quantity;      ///< amount of position
    std::optional<TimeStamp> open_time;    ///< time when position has been opened
    std::optional<Price> open_price;       ///< open price (can be aggregated)
    std::optional<Quantity> initial_margin;        ///< amount of money blocked for this position
    std::optional<Quantity> maintenance_margin;    ///< amount of money blocked for maintenance margin
};

using Positions = std::vector<Position>;


}