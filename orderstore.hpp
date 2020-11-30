#ifndef ORDERSTORE_HPP
#define ORDERSTORE_HPP

#include "messages.hpp"

class OrderStore
{
public:
    enum class OrderResponse
    {
        NA = 0,
        ACCEPT,
        REJECT
    };
    OrderStore();
    OrderResponse consume(const Message * message);
};

#endif // ORDERSTORE_HPP
