#ifndef SERVER_HPP
#define SERVER_HPP

#include "../messages.hpp"
#include "orderstore.hpp"
#include "../parser.hpp"

#include <sys/socket.h>
#include <string>
#include <unordered_map>

class Server
{
public:
    Server(uint64_t max_buy, uint64_t max_sell);
    ~Server();

    void start();

private:
    static const uint16_t INTERNET_PROTOCOL = AF_INET; // IPv4
    static const uint16_t TRANSPORT_PROTOCOL = SOCK_STREAM; // TCP
    static const uint16_t PORT_NUMBER = 1234;
    static const uint16_t PROTOCOL_VERSION = 1;

    static const uint16_t MAX_CONCURRENT_CLIENTS = 5;
    static const uint16_t BUFFER_SIZE = 64;

    Parser parser_{PROTOCOL_VERSION};
    std::unordered_map<int, std::unique_ptr<OrderStore>> clients_;
    uint64_t max_buy_;
    uint64_t max_sell_;

    int socket_ = -1;
    uint32_t sequence_number_ = 0;
};

#endif //REPO_SERVER_HPP
