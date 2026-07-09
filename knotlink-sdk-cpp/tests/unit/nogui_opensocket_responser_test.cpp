// unit test: OpenSocketResponser (nogui)
#include "../../knotlink/OpenSocketResponser.hpp"
#include <iostream>
#include <thread>
#include <chrono>
using namespace knotlink;

int main() {
    OpenSocketResponser res("app.knotlinksdk.test", "test_socket");
    res.setQuestionHandler([](const std::string& data) -> std::string {
        std::cout << "Received: " << data << std::endl;
        return "Echo: " + data;
    });
    std::cout << "Responser running..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(60));
    return 0;
}
