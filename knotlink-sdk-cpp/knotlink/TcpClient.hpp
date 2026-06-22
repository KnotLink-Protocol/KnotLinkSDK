#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class TcpClient {
public:
	TcpClient() : tcpSocket(INVALID_SOCKET), running(false) {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cerr << "WSAStartup failed." << std::endl;
			exit(1);
		}
	}
	
	~TcpClient() {
		stopHeartbeat();
		running = false;
		if (readThread.joinable()) {
			// 通过关闭 socket 打断 recv
			if (tcpSocket != INVALID_SOCKET) {
				closesocket(tcpSocket);
			}
			readThread.join();
		}
		if (tcpSocket != INVALID_SOCKET) {
			closesocket(tcpSocket);
		}
		WSACleanup();
	}
	
	bool connectToServer(const std::string& ip, uint16_t port) {
		tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (tcpSocket == INVALID_SOCKET) {
			std::cerr << "Failed to create socket." << std::endl;
			return false;
		}
		
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
		
		if (connect(tcpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			std::cerr << "Failed to connect to server." << std::endl;
			closesocket(tcpSocket);
			tcpSocket = INVALID_SOCKET;
			return false;
		}
		
		running = true;
		readThread = std::thread(&TcpClient::readData, this);
		startHeartbeat();
		return true;
	}
	
	// 发送数据（自动添加长度前缀）
	void sendData(const std::string& data) {
		if (tcpSocket == INVALID_SOCKET) {
			std::cerr << "Socket is not connected." << std::endl;
			return;
		}
		// 构造长度前缀（4字节大端）
		uint32_t len = static_cast<uint32_t>(data.size());
		uint32_t netLen = htonl(len); // 转换为网络字节序（大端）
		// 发送长度前缀
		int bytesSent = send(tcpSocket, (const char*)&netLen, sizeof(netLen), 0);
		if (bytesSent == SOCKET_ERROR) {
			handleError(WSAGetLastError());
			return;
		}
		// 发送数据
		bytesSent = send(tcpSocket, data.c_str(), data.size(), 0);
		if (bytesSent == SOCKET_ERROR) {
			handleError(WSAGetLastError());
		}
	}
	
	void startHeartbeat() {
		heartBeatThread = std::thread(&TcpClient::sendHeartbeat, this);
	}
	
	void stopHeartbeat() {
		if (heartBeatThread.joinable()) {
			heartBeatThread.join();
		}
	}
	
	void setOnDataReceivedCallback(const std::function<void(const std::string&)>& callback) {
		onDataReceivedCallback = callback;
	}
	
	bool running;
	
private:
	SOCKET tcpSocket;
	std::thread heartBeatThread;
	std::thread readThread;
	std::string heartbeatMessage = "heartbeat";
	std::string heartbeatResponse = "heartbeat_response";
	std::function<void(const std::string&)> onDataReceivedCallback;
	
	// 接收缓冲区（处理粘包）
	std::vector<char> recvBuffer;
	std::mutex recvMutex;
	static constexpr uint32_t MAX_MSG_SIZE = 16 * 1024 * 1024; // 16MB
	
	void sendHeartbeat() {
		while (running) {
			std::this_thread::sleep_for(std::chrono::seconds(180));
			if (running) {
				sendData(heartbeatMessage);
			}
		}
	}
	
	void handleError(int socketError) {
		std::cerr << "Socket error: " << socketError << std::endl;
		running = false;
	}
	
	// 解析缓冲区，提取完整消息
	void processBuffer() {
		while (true) {
			if (recvBuffer.size() < 4) break; // 长度字段未完整
			
			// 读取长度（大端）
			uint32_t netLen;
			memcpy(&netLen, recvBuffer.data(), 4);
			uint32_t len = ntohl(netLen);
			if (len == 0 || len > MAX_MSG_SIZE) {
				std::cerr << "Invalid message length: " << len << ", disconnecting." << std::endl;
				running = false;
				closesocket(tcpSocket);
				return;
			}
			if (recvBuffer.size() < 4 + len) break; // 消息体未完整
			
			// 提取消息体
			std::string msg(recvBuffer.data() + 4, len);
			// 移除已处理的数据
			recvBuffer.erase(recvBuffer.begin(), recvBuffer.begin() + 4 + len);
			
			// 处理消息：忽略心跳响应
			if (msg == heartbeatResponse) {
				std::cout << "Received heartbeat response, ignored." << std::endl;
			} else {
				if (onDataReceivedCallback) {
					onDataReceivedCallback(msg);
				}
			}
			// 继续循环处理可能存在的更多完整消息
		}
	}
	
	void readData() {
		char chunk[4096];
		while (running) {
			int bytesRead = recv(tcpSocket, chunk, sizeof(chunk), 0);
			if (bytesRead > 0) {
				std::lock_guard<std::mutex> lock(recvMutex);
				recvBuffer.insert(recvBuffer.end(), chunk, chunk + bytesRead);
				processBuffer();
			} else if (bytesRead == 0) {
				std::cout << "Server disconnected." << std::endl;
				running = false;
				break;
			} else {
				int err = WSAGetLastError();
				if (err == WSAECONNRESET || err == WSAECONNABORTED) {
					std::cout << "Connection reset." << std::endl;
				} else {
					handleError(err);
				}
				running = false;
				break;
			}
		}
	}
};

#endif // TCP_CLIENT_HPP
