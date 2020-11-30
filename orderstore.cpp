#include "orderstore.hpp"

#include <iostream>
#include <variant>

OrderStore::OrderStore()
{
}

// Ref.: https://en.cppreference.com/w/cpp/utility/variant/variant
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

auto OrderStore::consume(const Message * message) -> OrderResponse
{
    std::visit(overload{
        [](auto && arg) { std::cout << "   Type: " << arg.messageType << "\n"; }
    }, message->payload);
    return OrderResponse::NA;
}
