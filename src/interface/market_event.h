#pragma once

namespace quarkbot
{
    enum class MarketEvent {
        ticker,     /// < last ticker value
        orderbook,  /// < last orderbook state
        trade,      /// < last trade
        funding,    /// < last funding 
        instrument_info        /// < general information about the instrument
    };



} // namespace quarkbot



