/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef OPEN_SOCKET_QUERIER_HPP
#define OPEN_SOCKET_QUERIER_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "TcpClient.hpp"

class OpenSocketQuerier {
public:
    OpenSocketQuerier() {
        KLquerier = new TcpClient();
        KLquerier->connectToServer("127.0.0.1", 6376);
        KLquerier->setOnDataReceivedCallback(
            std::bind(&OpenSocketQuerier::handleReceivedData, this, std::placeholders::_1));
        while (!KLquerier->running) { /* wait connect */ }
    }

    ~OpenSocketQuerier() {
        KLquerier->stopHeartbeat();
        delete KLquerier;
    }

    void setConfig(const std::string& APPID, const std::string& OpenSocketID) {
        appID        = APPID;
        openSocketID = OpenSocketID;
    }

    // 同步阻塞提问
    std::string query_l(const std::string& question) {
        std::unique_lock<std::mutex> lock(mtx_);
        std::string qid = "1";
        std::string packet = appID + "-" + openSocketID + "&*&" + question;
        KLquerier->sendData(packet);

        cv_.wait(lock, [this, &qid] { return answers_.count(qid); });
        std::string ans = answers_["1"];
        answers_.erase("1");
        return ans;
    }

private:
    TcpClient* KLquerier;
    std::string appID;
    std::string openSocketID;

    std::mutex mtx_;
    std::condition_variable cv_;
    std::unordered_map<std::string, std::string> answers_;
    uint64_t questionCounter_ = 0;

    void handleReceivedData(const std::string& data) {
		std::cout<<data<<std::endl;

        std::lock_guard<std::mutex> lock(mtx_);
        answers_["1"] = data;
        cv_.notify_all();
    }
};

#endif // OPEN_SOCKET_QUERIER_HPP
