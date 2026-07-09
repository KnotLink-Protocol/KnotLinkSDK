/*
 * KnotLink SDK - C++ Integration Test
 * Request-Response pattern: Responser + Querier
 */

#include "../knotlink/OpenSocketResponser.hpp"
#include "../knotlink/OpenSocketQuerier.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
using namespace knotlink;

static std::atomic<bool> shutdown{false};

void run_responser() {
    OpenSocketResponser res("app.knotlinksdk.test", "test_socket");
    res.setQuestionHandler([](const std::string& data) -> std::string {
        std::cout << "[Responser] Received: " << data << std::endl;
        return "Echo: " + data;
    });
    std::cout << "[Responser] Running..." << std::endl;
    while (!shutdown) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void run_querier() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    OpenSocketQuerier q;
    q.setConfig("app.knotlinksdk.test", "test_socket");
    try {
        std::string result = q.query_l("Hello, Responser!");
        std::cout << "[Querier] Response: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Querier] Error: " << e.what() << std::endl;
    }
    shutdown = true;
}

int main() {
    std::thread t(run_responser);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    run_querier();
    t.join();
    return 0;
}
