#include "client.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <iostream>

Client::Client()
{
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == -1)
        throw std::runtime_error("Error creating client socket");

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = INTERNET_PROTOCOL;
    serv_addr.sin_port = htons(PORT_NUMBER);
    inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr);

    auto sock_addr = reinterpret_cast<sockaddr*>(&serv_addr);
    auto sock_len = socklen_t{sizeof(serv_addr)};
    auto server_con = connect(server_socket_, sock_addr, sock_len);
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
    sleep(1);

    char buffer[BUFFER_SIZE] = {};
    read(server_socket_, buffer, BUFFER_SIZE);
    auto msg = parser_.decode(buffer);
    if (std::holds_alternative<Messages::OrderResponse>(msg.payload)) {
        auto response = std::get<Messages::OrderResponse>(msg.payload);
        auto status = response.status == Messages::OrderResponse::Status::ACCEPTED ? "ACCEPTED" : "REJECTED";
        std::cout << "Status: " << status << " OrderId: " << response.orderId << "\n";
    }
}
