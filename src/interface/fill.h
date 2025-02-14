#pragma once

#include <memory>
#include "wrapper.h"

namespace quarkbot {


struct MarketFillInfo {     
    MarketType market_type;         ///< specifies on which market the fill has been received
    double quantity_multiplier;     ///< specifies quantity multiplier to correctly calculate PNL
    double price_multiplier;        ///< specifies price multiplier to correctly calculate PNL
};

struct Fill {
    MarketFillInfo market_info;     ///< short information about market
    TimeStamp time;                 ///< time of execution
    Price price;                    ///< price of execution
    Quantity quantity;              ///< executed quantity
    Side side;                      ///< side of execution
    double commision;               ///< commision 
    std::string fill_id;            ///< unique identifier of fill
    std::string position_id;        ///< unique identifier of position (if exists)
    std::string label;              ///< label (copied from other)
};

    


}