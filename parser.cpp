#include "parser.hpp"

#include <stdexcept>
#include <cstring>

Parser::Parser(uint16_t protocol_version)
    : protocol_version_(protocol_version)
{
}

Message Parser::decode(const char * data)
{
    size_t header_size = 16;
    Messages::Header header{};
    std::memcpy(&header, &data[0], header_size);
    if (header.version != protocol_version_)
        throw std::runtime_error("Unsupported protocol version");

    uint16_t messageType;
    std::memcpy(&messageType, &data[header_size], 2);

    Messages::Payload payload{};
    switch (messageType) {
        case Messages::NewOrder::MESSAGE_TYPE:
            payload = Messages::NewOrder{};
            break;
        case Messages::DeleteOrder::MESSAGE_TYPE:
            payload = Messages::DeleteOrder{};
            break;
        case Messages::ModifyOrderQuantity::MESSAGE_TYPE:
            payload = Messages::ModifyOrderQuantity{};
            break;
        case Messages::Trade::MESSAGE_TYPE:
            payload = Messages::Trade{};
            break;
        case Messages::OrderResponse::MESSAGE_TYPE:
            payload = Messages::OrderResponse{};
            break;
        default:
            throw std::runtime_error("Unsupported message type");
    }
    std::memcpy(&payload, &data[header_size], header.payloadSize);

    auto message = Message{header, payload};
    return message;
}