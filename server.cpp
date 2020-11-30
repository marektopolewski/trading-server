#include "server.hpp"

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <variant>
#include <iostream>

namespace
{
void make_local_address(struct sockaddr_in * addr, uint16_t port)
{
    addr->sin_family = AF_INET; // IPv4
    addr->sin_port = htons(port); // port in net-byte order
    addr->sin_addr.s_addr = INADDR_ANY; // bind to all interfaces
}
} // unnamed namespace

Server::Server()
{
    auto conn_socket = socket(INTERNET_PROTOCOL, TRANSPORT_PROTOCOL, 0);
    if (conn_socket == -1)
        throw std::runtime_error("Master socket not created");

    // generate a local address
    auto address = sockaddr_in{};
    make_local_address(&address, PORT_NUMBER);
    auto address_length = sizeof(address);
    auto sock_address = reinterpret_cast<sockaddr*>(&address);

    // bind the socket to the local address
    auto bind_success = bind(conn_socket, sock_address, address_length);
    if (bind_success == -1)
        throw std::runtime_error("Could not bind to the local machine");

    // listen on the bound address
    auto listen_success = listen(conn_socket, BACKLOG_SIZE);
    if (listen_success == -1)
        throw std::runtime_error("Could not listen on the local address");

    auto client_addr = reinterpret_cast<sockaddr*>(new sockaddr_storage{});
    auto client_len = new socklen_t{sizeof(client_addr)};
    client_socket_ = accept(conn_socket, client_addr, client_len);
    close(conn_socket);

    if (client_socket_ == -1)
        throw std::runtime_error("Could not create a socket");

    // Keep accepting requests until terminated
    active_ = true;
    while(active_) {
        char buffer[BUFFER_SIZE] = {};
        auto bytes_received = read(client_socket_, buffer, BUFFER_SIZE);
        if (bytes_received == 0)
            break;

        auto msg = "";
        if (strlen(buffer) == 0)
            msg = "404: No data";
        else {
//            uint16_t messageType;
//            memcpy(&messageType, &buffer[0], 2);
            auto data = reinterpret_cast<Message*>(buffer);
            orderStore_.consume(data);
            msg = "200: OK";
        }
        send(client_socket_, msg, sizeof(msg), 0);
    }
}

Server::~Server()
{
    close(client_socket_);
}

void Server::start()
{
    if (active_)
        return;
    active_ = true;


}

void Server::stop()
{
    if (active_)
        active_ = false;
}
