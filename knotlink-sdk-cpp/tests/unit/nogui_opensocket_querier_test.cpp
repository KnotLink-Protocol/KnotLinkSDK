// unit test: OpenSocketQuerier (nogui)
#include "../../knotlink/OpenSocketQuerier.hpp"
#include <iostream>
using namespace knotlink;

int main() {
    OpenSocketQuerier q;
    q.setConfig("app.knotlinksdk.test", "test_socket");
    try {
        std::string result = q.query_l("Hello, Responser!");
        std::cout << "Response: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
