#include "financialintrument.hpp"

#include <iostream>
#include <numeric>

namespace
{
    auto SUM_FUNC = [](const int acc, const std::pair<uint64_t, FinancialInstrument::Order> it) {
        return acc + it.second.quantity;
    };
} // unnamed namespace

void FinancialInstrument::add_buy(Order && order)
{
    buy_orders_[order.id] = order;
    update_buy();
}

void FinancialInstrument::add_sell(Order && order)
{
    sell_orders_[order.id] = order;
    update_sell();
}

void FinancialInstrument::add_trade(Order && order)
{
    trade_orders_[order.id] = order;
    net_pos_ = std::accumulate(trade_orders_.cbegin(), trade_orders_.cend(), 0, SUM_FUNC);
}

bool FinancialInstrument::delete_order(uint64_t id)
{
    auto buy_order = buy_orders_.find(id);
    if (buy_order != buy_orders_.end()) {
        buy_orders_.erase(buy_order);
        update_buy();
        return true;
    }

    auto sell_order = sell_orders_.find(id);
    if (sell_order != sell_orders_.end()) {
        sell_orders_.erase(sell_order);
        update_sell();
        return true;
    }

    return false;
}

bool FinancialInstrument::modify_order(uint64_t id, uint64_t quantity, uint64_t max_buy, uint64_t max_sell)
{
    auto buy_order = buy_orders_.find(id);
    if (buy_order != buy_orders_.end()) {
        if (quantity >= max_buy)
            throw std::logic_error("Exceeded max buy quantity threshold");
        buy_orders_[id].quantity = quantity;
        update_buy();
        return true;
    }

    auto sell_order = sell_orders_.find(id);
    if (sell_order != sell_orders_.end()) {
        if (quantity >= sell_qty_)
            throw std::logic_error("Exceeded max sell quantity threshold");
        sell_orders_[id].quantity = quantity;
        update_sell();
        return true;
    }

    return false;
}

void FinancialInstrument::update_buy()
{
    buy_qty_ = std::accumulate(buy_orders_.cbegin(), buy_orders_.cend(), 0, SUM_FUNC);
    buy_side_ = std::max(buy_qty_, net_pos_ + buy_qty_);
}

void FinancialInstrument::update_sell()
{
    sell_qty_ = std::accumulate(sell_orders_.cbegin(), sell_orders_.cend(), 0, SUM_FUNC);
    sell_side_ = std::max(sell_qty_, sell_qty_ - net_pos_);
}

void FinancialInstrument::update_trade()
{
    net_pos_ = std::accumulate(trade_orders_.cbegin(), trade_orders_.cend(), 0, SUM_FUNC);
}

void FinancialInstrument::test_print() const
{
    std::cout << "    " << "net_pos" << "   : " << net_pos_ << "\n";
    std::cout << "    " << "buy_qty" << "   : " << buy_qty_ << "\n";
    std::cout << "    " << "sell_qty" << "  : " << sell_qty_ << "\n";
    std::cout << "    " << "buy_side" << "  : " << buy_side_ << "\n";
    std::cout << "    " << "sell_side" << " : " << sell_side_ << "\n";
}
