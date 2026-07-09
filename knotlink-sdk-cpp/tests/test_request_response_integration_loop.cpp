// loop test: Request-Response (Querier + Responser)
#include "../knotlink/OpenSocketResponser.hpp"
#include "../knotlink/OpenSocketQuerier.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
using namespace knotlink;

static std::atomic<bool> shutdown{false};
constexpr int MAX_ITERATIONS = 5;

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
    for (int i = 1; i <= MAX_ITERATIONS && !shutdown; i++) {
        try {
            std::string data = "Hello #" + std::to_string(i);
            std::string result = q.query_l(data);
            std::cout << "[Querier] Req: " << data << " -> Res: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Querier] Error: " << e.what() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
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
