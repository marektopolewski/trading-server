#include "server.hpp"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <netinet/in.h>
#include <unistd.h>
#include <variant>

#include <iostream>
#include <arpa/inet.h>

namespace
{
void make_local_address(struct sockaddr_in * addr, uint16_t port)
{
    addr->sin_family = AF_INET; // IPv4
    addr->sin_port = htons(port); // port in net-byte order
    addr->sin_addr.s_addr = INADDR_ANY; // bind to all interfaces
}

uint64_t timestamp() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
}
} // unnamed namespace

Server::Server(uint64_t max_buy, uint64_t max_sell)
    : max_buy_(max_buy)
    , max_sell_(max_sell)
{
    socket_ = socket(INTERNET_PROTOCOL, TRANSPORT_PROTOCOL, 0);
    if (socket_ == -1)
        throw std::runtime_error("Master socket not created");

    // generate a local address
    auto address = sockaddr_in{};
    make_local_address(&address, PORT_NUMBER);
    auto address_length = sizeof(address);
    auto sock_address = reinterpret_cast<sockaddr*>(&address);

    // bind the socket to the local address
    auto bind_success = bind(socket_, sock_address, address_length);
    if (bind_success == -1)
        throw std::runtime_error("Could not bind to the local machine");

    // listen on the bound address and restrict max number of active clients
    auto listen_success = listen(socket_, 3);
    if (listen_success == -1)
        throw std::runtime_error("Could not listen on the local address");
}

Server::~Server()
{
    for (auto & client : clients_)
        close(client.first);
    close(socket_);
}

void Server::start()
{
    auto client_address = sockaddr_in{};
    make_local_address(&client_address, PORT_NUMBER);
    auto client_len = sizeof(client_address);

    fd_set socket_set;
    int client_sockets[MAX_CONCURRENT_CLIENTS];
    for (int & client_socket : client_sockets)
        client_socket = 0;

    while(true)
    {
        // Reset the socket set
        FD_ZERO(&socket_set);
        FD_SET(socket_, &socket_set);

        // Add valid client sockets to the set
        int last_active_socket = socket_;
        for (int client_socket : client_sockets) {
            if(client_socket > 0) {
                FD_SET(client_socket, &socket_set);
                last_active_socket = client_socket;
            }
        }

        auto timeout = timeval{5 * 60, 0}; // timeout after 5 minutes of no activity
        auto active_sockets = select(last_active_socket + 1, &socket_set, nullptr, nullptr, &timeout);
        if (active_sockets == -1)
            throw std::runtime_error("No active sockets found");

        // Handle new connections
        if (FD_ISSET(socket_, &socket_set)) {
            auto new_socket = accept(socket_, (sockaddr*)&client_address, (socklen_t*)&client_len);
            if (new_socket == -1)
                throw std::runtime_error("Could not establish connection with the client");

            // Add the client to the first empty socket slot
            for (auto & client_socket : client_sockets) {
                if(client_socket == 0) {
                    client_socket = new_socket;
                    clients_[client_socket] = std::make_unique<OrderStore>(max_buy_, max_sell_);
                    break;
                }
            }
        }

        // Handle reads from existing connections
        for (int & client_socket : client_sockets) {
            if (FD_ISSET(client_socket, &socket_set)) {
                char buffer[BUFFER_SIZE] = {};
                auto bytes_received = read(client_socket, buffer, BUFFER_SIZE);

                // If the client connection was terminated
                if (bytes_received == 0) {
                    getpeername(client_socket , (struct sockaddr*)&client_address , (socklen_t*)&client_len);
                    clients_.erase(client_socket);
                    close(client_socket);
                    client_socket = 0;
                }

                // If a new message incoming - parse and handle in the OrderStore
                else {
                    auto message = parser_.decode(buffer);
                    auto response = clients_[client_socket]->consume(std::move(message));
                    if (!response.no_response) {
                        auto msg = Message{};
                        msg.header = { PROTOCOL_VERSION, sizeof(Messages::OrderResponse),
                                       sequence_number_++, timestamp() };
                        msg.payload = Messages::OrderResponse{Messages::OrderResponse::MESSAGE_TYPE,
                                                              response.order_id, response.status};
                        send(client_socket, &msg, sizeof(msg), 0);
                    }
                }
            }
        }
    }
}

// Ref.: https://beej.us/guide/bgnet/html//index.html
/*
void start()
{
    // Initialise the socket set
    auto master = fd_set{};
    FD_ZERO(&master);
    FD_SET(socket_, &master);

    for (;;) {
        auto copy = master;
        auto socket_count = select(0, &copy, nullptr, nullptr, nullptr);
        if (socket_count == -1)
            break;

        // Iterate connected socket
        for (int socket = 0; socket < MAX_CONCURRENT_CLIENTS + 1; ++socket) {

            if (!FD_ISSET(socket, &copy)) {
                std::cerr << "Socket " << socket << "not set\n";
                continue;
            }

            // If visiting the master socket, then try to accept new connections
            if (socket == socket_) {
                std::cerr << "Accepting at the master socket" << std::endl;
                auto client_socket = accept(socket_, nullptr, nullptr);
                if (client_socket != -1)
                    FD_SET(client_socket, &master);
            }
            // If visiting a client socket, then listen for incoming messages
            else {
                std::cerr << "Client socket #" << socket << std::endl;
                char buffer[BUFFER_SIZE] = {};
                auto bytes_received = read(socket, buffer, BUFFER_SIZE);

                // Handle closed connection
                if (bytes_received <= 0) {
                    clients_.erase(socket);
                    close(socket);
                    FD_CLR(socket, &master);
                }
                // Otherwise parse and handle the message
                else {
                    auto message = parser_.decode(buffer);
                    auto response = orderStore_.consume(std::move(message));
                    if (response.no_response)
                        continue;

                    auto msg = Message{};
                    msg.header = {PROTOCOL_VERSION, sizeof(Messages::OrderResponse), sequence_number_++, timestamp()};
                    msg.payload = Messages::OrderResponse{ Messages::OrderResponse::MESSAGE_TYPE,
                                                           response.order_id, response.status };
                    send(socket, &msg, sizeof(msg), 0);
                }
            }
        }
    }
}*/
