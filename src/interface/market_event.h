#pragma once
#include "flagmap.h"

namespace quarkbot
{
    enum class MarketEvent {
        end_of_stream,          ///< always sent when stream is closed
        ticker,                 ///< last ticker value
        orderbook,              ///< last orderbook state
        trade,                  ///< last trade
        funding,                ///< last funding
        index,                  ///< index
        instrument_info         ///< general information about the instrument
    };

    using MarketEvents = FlagMap<MarketEvent>;

} // namespace quarkbot



