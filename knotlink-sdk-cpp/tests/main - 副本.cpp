#include "SignalSubscriber.hpp"
#include "SignalSender.hpp"
#include <chrono>
#include <thread>

void onDataReceived(const std::string& data) {
	std::cout << "Received data:q " << data << std::endl;
}

int main() {
	SignalSubscriber *subscriber =new SignalSubscriber("0x00000001", "0x00000001");
	
	subscriber->setOnDataReceivedCallback(onDataReceived);
	
	SignalSender *sd = new SignalSender("0x00000001", "0x00000001");
	
	// Keep the program running
	while(1){
		sd->emitt("nh");
		std::cout<<111;
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return 0;
	}
		
	
	std::cout<<333;
	
//	subscriber.stop();
	
	std::cout<<444;
	
//	sd.~SignalSender();
	
	std::cout<<444;
		return 0;
	
	
	return 0;
}

