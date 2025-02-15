#include "subscriber.h"
#include <functional>
namespace quarkbot {



template class Subscriber<std::function<void(Instrument, MarketEvents)> >;

}

