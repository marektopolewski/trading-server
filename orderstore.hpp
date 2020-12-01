#ifndef ORDERSTORE_HPP
#define ORDERSTORE_HPP

#include "financialintrument.hpp"
#include "messages.hpp"

#include <unordered_map>

class OrderStore
{
public:
    enum class OrderResponse
    {
        NA = 0,
        ACCEPT,
        REJECT
    };
    OrderStore(int max_buy, int max_sell);
    OrderResponse consume(Message && message);

protected:
    using IntrumentMap = std::unordered_map<uint64_t, FinancialInstrument>;
    IntrumentMap & test_instruments() { return instruments_; }

private:
    static const uint8_t PROTOCOL_VERSION = 1;

    IntrumentMap instruments_;
    int max_buy_;
    int max_sell_;
};

#endif // ORDERSTORE_HPP
