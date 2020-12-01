#include "orderstore.hpp"

#include <iostream>
#include <variant>

OrderStore::OrderStore(int max_buy, int max_sell)
    : max_buy_(max_buy)
    , max_sell_(max_sell)
{
}

// Ref.: https://en.cppreference.com/w/cpp/utility/variant/variant
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

auto OrderStore::consume(Message && message) -> OrderResponse
{
    if (message.header.version != PROTOCOL_VERSION) {
        std::cerr << "Invalid protocol version, message will be ignored\n";
        return OrderResponse::NA;
    }

    auto response = OrderResponse::NA;
    std::visit(overload{
        [&](Messages::NewOrder payload) { response = handle_add(std::move(payload)); },
        [&](Messages::DeleteOrder payload) { response = handle_delete(std::move(payload)); },
        [&](Messages::ModifyOrderQuantity payload) { response = handle_modify(std::move(payload)); },
        [](auto arg) { throw std::runtime_error("Unsupported message type"); }

    }, message.payload);

    return response;
}

auto OrderStore::handle_add(Messages::NewOrder && payload) -> OrderResponse
{
    auto & instrument = instruments_[payload.listingId];

    if (payload.side == 'B') {
        if (payload.orderQuantity + instrument.buy_side() >= max_buy_)
            return OrderResponse::REJECT;
        instrument.add_buy({payload.orderId, payload.orderQuantity, payload.orderPrice});
    }

    else if (payload.side == 'S') {
        if (payload.orderQuantity + instrument.sell_side() >= max_sell_)
            return OrderResponse::REJECT;
        instrument.add_sell({payload.orderId, payload.orderQuantity, payload.orderPrice});
    }

    return OrderResponse::ACCEPT;
}

auto OrderStore::handle_delete(Messages::DeleteOrder && payload) -> OrderResponse
{
    for (auto &instrument : instruments_) {
        if (instrument.second.delete_order(payload.orderId))
            return OrderResponse::ACCEPT;
    }
    return OrderResponse::REJECT;
}

auto OrderStore::handle_modify(Messages::ModifyOrderQuantity && payload) -> OrderResponse
{
    if (payload.newQuantity == 0)
        return OrderResponse::REJECT;

    try {
        for (auto &instrument : instruments_) {
            if (instrument.second.modify_order(payload.orderId, payload.newQuantity, max_buy_, max_sell_))
                return OrderResponse::ACCEPT;
        }
    }
    catch (const std::logic_error &) { /* threshold exceeded */ }

    return OrderResponse::REJECT;
}

