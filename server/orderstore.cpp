#include "orderstore.hpp"

#include <variant>

using OrderStatus = Messages::OrderResponse::Status;

OrderStore::OrderStore(int max_buy, int max_sell)
    : max_buy_(max_buy)
    , max_sell_(max_sell)
{
}

// Ref.: https://en.cppreference.com/w/cpp/utility/variant/variant
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

auto OrderStore::consume(Message && message) -> Response
{
    auto response = Response{};
    std::visit(overload{
        [&](Messages::NewOrder payload) { response = handle_add(std::move(payload)); },
        [&](Messages::DeleteOrder payload) { response = handle_delete(std::move(payload)); },
        [&](Messages::ModifyOrderQuantity payload) { response = handle_modify(std::move(payload)); },
        [&](Messages::Trade payload) { response = handle_trade(std::move(payload)); },
        [](auto arg) { throw std::runtime_error("Unsupported message type"); }

    }, message.payload);
    return response;
}

auto OrderStore::handle_add(Messages::NewOrder && payload) -> Response
{
    auto & instrument = instruments_[payload.listingId];
    try {
        auto signed_quantity = static_cast<int64_t>(payload.orderQuantity);
        if (payload.side == 'B')
            instrument.add_buy({payload.orderId, signed_quantity, payload.orderPrice}, max_buy_);
        else if (payload.side == 'S')
            instrument.add_sell({payload.orderId, signed_quantity, payload.orderPrice}, max_sell_);
        return { OrderStatus::ACCEPTED, payload.orderId };
    }
    catch (const std::logic_error &) { /* threshold exceeded */ }
    return { OrderStatus::REJECTED, payload.orderId };
}

auto OrderStore::handle_delete(Messages::DeleteOrder && payload) -> Response
{
    try {
        for (auto & instrument : instruments_) {
            if (instrument.second.delete_order(payload.orderId))
                return { OrderStatus::ACCEPTED, payload.orderId };
        }
    }
    catch (const std::logic_error &) { /* threshold exceeded */ }
    return { OrderStatus::REJECTED, payload.orderId };
}

auto OrderStore::handle_modify(Messages::ModifyOrderQuantity && payload) -> Response
{
    if (payload.newQuantity == 0)
        return { OrderStatus::REJECTED, payload.orderId };
    try {
        for (auto & instrument : instruments_) {
            if (instrument.second.modify_order(payload.orderId, payload.newQuantity, max_buy_, max_sell_))
                return { OrderStatus::ACCEPTED, payload.orderId };
        }
    }
    catch (const std::logic_error &) { /* threshold exceeded */ }
    return { OrderStatus::REJECTED, payload.orderId };
}

auto OrderStore::handle_trade(Messages::Trade && payload) -> Response
{
    if (payload.tradeQuantity == 0 || payload.tradePrice == 0)
        return { OrderStatus::REJECTED, payload.tradeId };
    auto & instrument = instruments_[payload.listingId];
    try {
        auto signed_quantity = static_cast<int64_t>(payload.tradeQuantity);
        instrument.add_trade({payload.tradeId, signed_quantity, payload.tradePrice}, max_buy_, max_sell_);
        return { OrderStatus::ACCEPTED, payload.tradeId };
    }
    catch (const std::logic_error &) { /* threshold exceeded or no matching trade found */ }
    return { OrderStatus::REJECTED, payload.tradeId };
}
