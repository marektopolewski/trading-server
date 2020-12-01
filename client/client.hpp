#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../messages.hpp"
#include "../parser.hpp"

#include <sys/socket.h>

class Client
{
public:
    Client();
    ~Client();
    void sendMessage(const Message & message);

private:
    static const uint16_t INTERNET_PROTOCOL = AF_INET; // IPv4
    inline static const char * ADDRESS = "127.0.0.1";
    static const uint16_t PORT_NUMBER = 1234;
    static const uint16_t PROTOCOL_VERSION = 1;

    static const uint16_t BUFFER_SIZE = 64;

    Parser parser_{PROTOCOL_VERSION};

    int server_socket_ = -1;
};

#endif // FLOW_CLIENT_HPP
