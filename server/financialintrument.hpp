#ifndef FINANCIALINTRUMENT_HPP
#define FINANCIALINTRUMENT_HPP

#include "../messages.hpp"

#include <unordered_map>

class FinancialInstrument
{
public:
    struct Order
    {
        uint64_t id;
        uint64_t quantity;
        uint64_t price;
    };
    void add_buy(Order && order, uint64_t max_buy);
    void add_sell(Order && order, uint64_t max_sell);
    void add_trade(Order && order, uint64_t max_buy, uint64_t max_sell);

    bool delete_order(uint64_t id);
    bool modify_order(uint64_t id, uint64_t quantity, uint64_t max_buy, uint64_t max_sell);

    using OrderMap = std::unordered_map<uint64_t, Order>;
    const OrderMap & trades() { return trade_orders_; }
    const OrderMap & buys() { return buy_orders_; }
    const OrderMap & sells() { return sell_orders_; }

private:
    void update_buy();
    void update_sell();
    void update_trade();

    int net_pos_;
    int buy_qty_;
    int sell_qty_;
    int buy_side_;
    int sell_side_;

    OrderMap trade_orders_;
    OrderMap buy_orders_;
    OrderMap sell_orders_;

    bool strict_ = false; // Defines if a trade must be matched with a buy/sell order. Intuitively, a long trade should
                          // have a corresponding buy order, while a short trade a sell order. The specification does
                          // not explicitly place such a constraint and neither is it present in the example, hence,
                          // this mode is disabled by default. See the `orderstore.DISABLED_trade_mismatch` test.
};

inline bool operator==(const FinancialInstrument::Order & lhs, const FinancialInstrument::Order & rhs) {
    return lhs.quantity == rhs.quantity && lhs.price == rhs.price;
}

#endif //FINANCIALINTRUMENT_HPP
