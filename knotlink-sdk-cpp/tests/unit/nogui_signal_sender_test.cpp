// unit test: SignalSender (nogui)
#include "../../knotlink/SignalSender.hpp"
#include <iostream>
using namespace knotlink;

int main() {
    SignalSender sender;
    sender.setConfig("app.knotlinksdk.test", "test_signal");
    sender.emitt("Hello from Sender!");
    std::cout << "Signal sent." << std::endl;
    return 0;
}
