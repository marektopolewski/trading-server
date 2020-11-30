#include "server.hpp"

#include <iostream>

int main()
{
    try {
        auto server = Server();
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