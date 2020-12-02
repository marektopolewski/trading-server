#include "client.hpp"

#include <chrono>
#include <iostream>
#include <unistd.h>

int main()
{
    using namespace std::chrono;
    try {
        auto client = Client();
        sleep(1);

        while (true) {
            std::string input;
            std::cout << "\nPress any key to resend.. ";
            std::cin >> input;
            auto ts = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
            auto hdr = Messages::Header{1, 35, static_cast<uint32_t>(1), static_cast<uint64_t>(ts)};
            auto pld = Messages::NewOrder{Messages::NewOrder::MESSAGE_TYPE, 1, static_cast<uint32_t>(1), 7, 3, 'B'};
            auto msg = Message{hdr, pld};
            client.sendMessage(msg);
            sleep(1);
        }
    }
    catch(const std::runtime_error & err) {
        std::cerr << "[ERR] " << err.what() << "\n";
    }
    catch(...) {
        std::cerr << "[ERR] Unknown error, exiting..\n";
    }
    return 0;
}