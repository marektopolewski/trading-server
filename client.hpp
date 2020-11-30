#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "messages.hpp"

class Client
{
public:
    Client();
    ~Client();
    void sendMessage(const Message & message);

private:
    int server_socket_ = -1;
};

#endif // FLOW_CLIENT_HPP
