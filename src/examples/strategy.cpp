#include "../interface/context.h"

using namespace quarkbot;


StrategyContext &create_context();

static bool order_is_same_price(Order ord, Price price) {
    if (ord.get_state() != OrderState::open) return true;
    auto setup = ord.get_setup_as<OrderType::limit>();
    if (!setup) return false;
    return setup->get_limit_price() == price;
}

coroutine<void> run_strategy(StrategyContext &ctx) {

    Instrument instr =ctx.find_instrument_by_label("main");
    auto info = instr.get_info();

    Order top = ctx.prepare_order(instr, "top");
    Order bottom = ctx.prepare_order(instr, "bottom");

    ctx.set_subscription(instr, MarketEvent::ticker);

    Quantity q = info.align_quantity(1, AlignmentStrategy::aggresive);

    while (true) {
        co_await ctx.on_market(instr);
        auto tk = instr.get_ticker();
        if (tk.has_value()) {
            Price top_price = tk->ask * 1.01;
            Price bottom_price = tk->bid / 1.01;
            top_price = info.align_price(top_price, AlignmentStrategy::defensive, Side::ask);
            bottom_price = info.align_price(bottom_price, AlignmentStrategy::defensive, Side::bid);
            if (!order_is_same_price(top, top_price)) {
                top.cancel();
            }
            if (!order_is_same_price(bottom, bottom_price)) {
                bottom.cancel();
            }
            if (top.is_done()) {
                top = top.replace(q, OrderType::limit(Side::ask, top_price));
            }
            if (bottom.is_done()) {
                bottom = bottom.replace(q, OrderType::limit(Side::bid, bottom_price));
            }
        }
    }



}



int main() {


}
