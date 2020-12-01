#include "client.hpp"
#include "messages.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <chrono>

const uint16_t PORT_NUMBER = 1234;
const char * ADDRESS = "127.0.0.1";

Client::Client()
{
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == -1)
        throw std::runtime_error("Error creating client socket");

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUMBER);
    inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr);

    auto server_con = connect(server_socket_, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (server_con == -1)
        throw std::runtime_error("Could not connect to the server");
}

Client::~Client()
{
    close(server_socket_);
}

void Client::sendMessage(const Message & message)
{
    send(server_socket_, &message, sizeof(message), 0);
    char buffer[1024] = {0};
    read(server_socket_, buffer, 1024);
    std::cout << "Server response: \"" << buffer << "\"\n";
}

//int main()
//{
//    using namespace std::chrono;
//    try {
//        auto client = Client();
//        auto ts = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
//        auto hdr = Messages::Header{ 1, 35, static_cast<uint32_t>(1), static_cast<uint64_t>(ts) };
//        auto pld = Messages::NewOrder{ Messages::NewOrder::MESSAGE_TYPE, 1, static_cast<uint32_t>(1), 7,  3, 'B' };
//        auto msg = Message{hdr, pld};
//        client.sendMessage(msg);
//        sleep(5);
//    }
//    catch(const std::runtime_error & err) {
//        std::cerr << "[ERR] " << err.what() << "\n";
//    }
//    catch(...) {
//        std::cerr << "[ERR] Unknown error, exiting..\n";
//    }
//    return 0;
//}
