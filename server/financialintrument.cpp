#include "financialintrument.hpp"

#include <numeric>

namespace
{
    auto SUM_FUNC = [](const int64_t acc, const std::pair<uint64_t, FinancialInstrument::Order> & it) {
        return acc + it.second.quantity;
    };
} // unnamed namespace

void FinancialInstrument::add_buy(Order && order, uint64_t max_buy)
{
    buy_orders_[order.id] = order;
    update_buy();
    if (buy_side_ >= max_buy) {
        buy_orders_.erase(order.id);
        update_buy();
        throw std::logic_error("Exceeded max buy quantity threshold");
    }
}

void FinancialInstrument::add_sell(Order && order, uint64_t max_sell)
{
    sell_orders_[order.id] = order;
    update_sell();
    if (sell_side_ >= max_sell) {
        sell_orders_.erase(order.id);
        update_sell();
        throw std::logic_error("Exceeded max sell quantity threshold");
    }
}

void FinancialInstrument::add_trade(Order && order, uint64_t max_buy, uint64_t max_sell)
{
    // Implicitly find the sign of the trade (negative for short, positive for long) by finding a corresponding
    // buy or sell order.
    auto buy_match = buy_orders_.find(order.id);
    if (buy_match != buy_orders_.end() && order == buy_match->second) {
        order.quantity = (inverted_ ? 1 : -1) * order.quantity;
    }
    else {
        auto sell_match = sell_orders_.find(order.id);
        if (sell_match != sell_orders_.end() && order == sell_match->second)
            order.quantity = (inverted_ ? -1 : 1) * order.quantity;
        else
            throw std::logic_error("No buy or sell order matching the trade");
    }
    trade_orders_[order.id] = order;
    update_trade();
    if (buy_side_ >= max_buy || sell_side_ >= max_sell) {
        trade_orders_.erase(order.id);
        update_trade();
        throw std::logic_error("Exceeded max buy or sell quantity threshold");
    }
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
        auto prev_quantity = buy_orders_[id].quantity;
        buy_orders_[id].quantity = quantity;
        update_buy();
        if (buy_side_ >= max_buy) {
            buy_orders_[id].quantity = prev_quantity;
            update_buy();
            throw std::logic_error("Exceeded max buy quantity threshold");
        }
        return true;
    }
    auto sell_order = sell_orders_.find(id);
    if (sell_order != sell_orders_.end()) {
        auto prev_quantity = sell_orders_[id].quantity;
        sell_orders_[id].quantity = quantity;
        update_sell();
        if (sell_side_ >= max_sell) {
            sell_orders_[id].quantity = prev_quantity;
            update_sell();
            throw std::logic_error("Exceeded max sell quantity threshold");
        }
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
    buy_side_ = std::max(buy_qty_, net_pos_ + buy_qty_);
    sell_side_ = std::max(sell_qty_, sell_qty_ - net_pos_);
}
