#include "server.hpp"

#include <iostream>

int main()
{
    try {
        auto server = Server();
        server.start();
    }
    catch(const char * errmsg) {
        std::cout << "[ERR] " << errmsg << "\n";
    }
    return 0;
}