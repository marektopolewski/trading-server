#include "server.hpp"

#include <iostream>

int main()
{
    try {
        std::string max_buy, max_sell;

        std::cout << "Enter max buy threshold: ";
        std::cin >> max_buy;

        std::cout << "Enter max sell threshold: ";
        std::cin >> max_sell;

        auto server = Server(std::stoull(max_buy), std::stoull(max_sell));
        server.start();
    }
    catch(const std::runtime_error & err) {
        std::cerr << "[ERR] " << err.what() << "\n";
    }
    catch(...) {
        std::cerr << "[ERR] Unknown error, exiting..\n";
    }
    return 0;
}