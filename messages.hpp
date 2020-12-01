#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <cstdint>
#include <variant>

namespace Messages
{

struct Header
{
    uint16_t version; 
    uint16_t payloadSize; 
    uint32_t sequenceNumber; 
    uint64_t timestamp; 
} __attribute__ ((__packed__));
static_assert(sizeof(Header) == 16, "The Header size is not correct");

struct NewOrder
{
    static constexpr uint16_t MESSAGE_TYPE = 1;
    uint16_t messageType; 
    uint64_t listingId; 
    uint64_t orderId; 
    uint64_t orderQuantity; 
    uint64_t orderPrice; 
    char side; 
} __attribute__ ((__packed__));
static_assert(sizeof(NewOrder) == 35, "The NewOrder size is not correct");

struct DeleteOrder
{
    static constexpr uint16_t MESSAGE_TYPE = 2;
    uint16_t messageType; 
    uint64_t orderId; 
} __attribute__ ((__packed__));
static_assert(sizeof(DeleteOrder) == 10, "The DeleteOrder size is not correct");

struct ModifyOrderQuantity
{
    static constexpr uint16_t MESSAGE_TYPE = 3;
    uint16_t messageType; 
    uint64_t orderId; 
    uint64_t newQuantity; 
} __attribute__ ((__packed__));
static_assert(sizeof(ModifyOrderQuantity) == 18, "The ModifyOrderQuantity size is not correct");

struct Trade
{
    static constexpr uint16_t MESSAGE_TYPE = 4;
    uint16_t messageType; 
    uint64_t listingId; 
    uint64_t tradeId; 
    uint64_t tradeQuantity; 
    uint64_t tradePrice; 
} __attribute__ ((__packed__));
static_assert(sizeof(Trade) == 34, "The Trade size is not correct");

struct OrderResponse
{
    static constexpr uint16_t MESSAGE_TYPE = 5;
    enum class Status : uint16_t
    {
        ACCEPTED = 0,
        REJECTED = 1,
    };
    uint16_t messageType; 
    uint64_t orderId; 
    Status status; 
} __attribute__ ((__packed__));
static_assert(sizeof(OrderResponse) == 12, "The OrderResponse size is not correct");

using Payload = std::variant<Messages::NewOrder, Messages::DeleteOrder,Messages::Trade,
                             Messages::ModifyOrderQuantity, Messages::OrderResponse>;
}

struct Message
{
    Messages::Header header;
    Messages::Payload payload;
};

#endif 
