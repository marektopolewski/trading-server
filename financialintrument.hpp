#ifndef FINANCIALINTRUMENT_HPP
#define FINANCIALINTRUMENT_HPP

#include "messages.hpp"

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
    void add_buy(Order && order);
    void add_sell(Order && order);
    void add_trade(Order && order);

    bool delete_order(uint64_t id);
    bool modify_order(uint64_t id, uint64_t quantity, uint64_t max_buy, uint64_t max_sell);

    void test_print() const;

private:
    void update_buy();
    void update_sell();
    void update_trade();

    int net_pos_;
    int buy_qty_;
    int sell_qty_;
    int buy_side_;
    int sell_side_;

    std::unordered_map<uint64_t, Order> trade_orders_;
    std::unordered_map<uint64_t, Order> buy_orders_;
    std::unordered_map<uint64_t, Order> sell_orders_;
};


#endif //FINANCIALINTRUMENT_HPP
