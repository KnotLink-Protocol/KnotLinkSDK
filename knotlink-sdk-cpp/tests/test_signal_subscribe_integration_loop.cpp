// loop test: Signal-Subscribe (Sender + Subscriber)
#include "../knotlink/SignalSubscriber.hpp"
#include "../knotlink/SignalSender.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
using namespace knotlink;

static std::atomic<bool> shutdown{false};
constexpr int MAX_ITERATIONS = 5;

void run_subscriber() {
    SignalSubscriber sub("app.knotlinksdk.test", "test_signal");
    sub.setOnDataReceivedCallback([](const std::string& data) {
        std::cout << "[Subscriber] Received: " << data << std::endl;
    });
    std::cout << "[Subscriber] Listening..." << std::endl;
    while (!shutdown) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void run_sender() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    SignalSender sender;
    sender.setConfig("app.knotlinksdk.test", "test_signal");
    for (int i = 1; i <= MAX_ITERATIONS && !shutdown; i++) {
        std::string data = "Signal #" + std::to_string(i);
        sender.emitt(data);
        std::cout << "[Sender] Sent: " << data << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    shutdown = true;
}

int main() {
    std::thread t(run_subscriber);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    run_sender();
    t.join();
    return 0;
}
