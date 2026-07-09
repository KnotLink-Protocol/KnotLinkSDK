// unit test: SignalSubscriber (nogui)
#include "../../knotlink/SignalSubscriber.hpp"
#include <iostream>
#include <thread>
#include <chrono>
using namespace knotlink;

int main() {
    SignalSubscriber sub("app.knotlinksdk.test", "test_signal");
    sub.setOnDataReceivedCallback([](const std::string& data) {
        std::cout << "Received: " << data << std::endl;
    });
    std::cout << "Subscriber listening..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));
    return 0;
}
