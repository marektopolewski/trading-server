#include "server.hpp"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

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
    socket_ = socket(INTERNET_PROTOCOL, TRANSPORT_PROTOCOL, 0);
    if (socket_ == -1)
        throw "Master socket not created";

    // generate a local address
    auto address = sockaddr_in{};
    make_local_address(&address, PORT_NUMBER);
    auto address_length = sizeof(address);
    auto sock_address = reinterpret_cast<sockaddr*>(&address);

    // bind the socket to the local address
    auto bind_success = bind(socket_, sock_address, address_length);
    if (bind_success == -1)
        throw "Could not bind to the local machine";

    // listen on the bound address
    auto listen_success = listen(socket_, BACKLOG_SIZE);
    if (listen_success == -1)
        throw "Could not listen on the local address";
}

void Server::start()
{
    if (active_) {
        std::cout << "Cannot start. Server already running.\n";
        return;
    }
    active_ = true;

    // Keep accepting requests until terminated
    while(active_) {
        auto client_addr = reinterpret_cast<sockaddr*>(new sockaddr_storage{});
        auto client_len = new socklen_t{sizeof(client_addr)};

        auto client_socket = accept(socket_, client_addr, client_len);
        if (client_socket == -1)
            throw "Could not create a socket";

        char buffer[BUFFER_SIZE] = {};
        read(client_socket, buffer, BUFFER_SIZE);
        std::cout << "Incoming:  \"" << buffer << "\"\n";

        auto msg = "";
        if (strlen(buffer) == 0)
            msg = "404: No data";
        else
            msg = "200: OK";
        send(client_socket, msg, sizeof(msg), 0);

        close(client_socket);
    }
}

void Server::stop()
{
    if (!active_)
        std::cout << "Cannot stop. Server not running.\n";
    active_ = false;
}
