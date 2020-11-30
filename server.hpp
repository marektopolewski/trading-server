#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <string>

class Server
{
public:
    Server();
    ~Server() {

    }
    void start();
    void stop();

private:
    static const uint16_t INTERNET_PROTOCOL = AF_INET; // IPv4
    static const uint16_t TRANSPORT_PROTOCOL = SOCK_STREAM; // TCP
    static const uint16_t PORT_NUMBER = 1234;

    static const uint16_t BACKLOG_SIZE = 5;
    static const uint16_t BUFFER_SIZE = 64;

    bool active_ = false;
    int socket_ = -1;
};

#endif //REPO_SERVER_HPP
