#ifndef ORDERSTORE_HPP
#define ORDERSTORE_HPP

#include "financialintrument.hpp"
#include "../messages.hpp"

#include <unordered_map>

class OrderStore
{
public:
    struct Response
    {
        Response(Messages::OrderResponse::Status status, uint64_t order_id)
            : status(status)
            , order_id(order_id)
            , no_response(false)
        {}
        Response() = default;
        Messages::OrderResponse::Status status;
        uint64_t order_id;
        bool no_response = true;
    };
    OrderStore(int max_buy, int max_sell);
    Response consume(Message && message);

protected:
    using IntrumentMap = std::unordered_map<uint64_t, FinancialInstrument>;
    IntrumentMap & test_instruments() { return instruments_; }

private:
    Response handle_add(Messages::NewOrder && payload);
    Response handle_delete(Messages::DeleteOrder && payload);
    Response handle_modify(Messages::ModifyOrderQuantity && payload);
    Response handle_trade(Messages::Trade && payload);

    IntrumentMap instruments_;
    int max_buy_;
    int max_sell_;
};

#endif // ORDERSTORE_HPP
