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

        [&](Messages::NewOrder arg) {
            if ((arg.side == 'B' && arg.orderQuantity >= max_buy_) ||
                (arg.side == 'S' && arg.orderQuantity >= max_sell_)) {
                response = OrderResponse::REJECT;
                return;
            }
            auto & instrument = instruments_[arg.listingId];
            if (arg.side == 'B')
                instrument.add_buy({arg.orderId, arg.orderQuantity, arg.orderPrice});
            else if (arg.side == 'S')
                instrument.add_sell({arg.orderId, arg.orderQuantity, arg.orderPrice});
            response = OrderResponse::ACCEPT;
        },

        [&](Messages::DeleteOrder arg) {
            for (auto & instrument : instruments_) {
                if (instrument.second.delete_order(arg.orderId)) {
                    response = OrderResponse::ACCEPT;
                    return;
                }
            }
            response = OrderResponse::REJECT;
        },

        [&](Messages::ModifyOrderQuantity arg) {
            if (arg.newQuantity == 0) {
                response = OrderResponse::REJECT;
                return;
            }
            try {
                for (auto &instrument : instruments_) {
                    if (instrument.second.modify_order(arg.orderId, arg.newQuantity, max_buy_, max_sell_)) {
                        response = OrderResponse::ACCEPT;
                        return;
                    }
                }
            }
            // logic error is thrown when the quantity threshold is exceeded
            catch (const std::logic_error & err) {}
            response = OrderResponse::REJECT;
        },

        [](auto arg) { throw std::runtime_error("Unsupported message type"); }

    }, message.payload);

    return response;
}


