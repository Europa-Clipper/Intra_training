#include "../include/ws_server.h"
#include <iostream>
#include <algorithm>
#include <atomic>

myWebSocketServer::myWebSocketServer(int port) 
    : m_port(port), m_server(port), m_running(false) {
    
    // 配置连接回调
    m_server.setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> wsWeakPtr, std::shared_ptr<ix::ConnectionState> state) {
            auto webSocket = wsWeakPtr.lock();
            if (!webSocket) return;
            
            // 添加客户端到连接列表
            {
                std::lock_guard<std::mutex> lock(m_clientsMutex);
                m_clients.push_back(webSocket);
                std::cout << "新客户端连接，当前连接数: " << m_clients.size() << std::endl;
            }
            
            // 设置消息回调
            webSocket->setOnMessageCallback(
                [this, webSocket](const ix::WebSocketMessagePtr& msg) {
                    if (msg->type == ix::WebSocketMessageType::Message) {
                        std::cout << "\n服务器收到消息: " << msg->str << std::endl;
                        std::cout << "服务器: 请输入消息发送(输入'exit'退出): " << std::endl;
                        
                        // 可以在这里添加自动回复逻辑
                        // webSocket->send("服务器已收到: " + msg->str);
                    }
                    else if (msg->type == ix::WebSocketMessageType::Close) {
                        std::cout << "\n客户端断开连接" << std::endl;
                        std::cout << "服务器: 请输入消息发送(输入'exit'退出): " << std::endl;
                        removeClient(webSocket);
                    }
                }
            );
        }
    );
}

myWebSocketServer::~myWebSocketServer() {
    stop();
}

bool myWebSocketServer::start() {
    auto res = m_server.listen();
    if (!res.first) {
        std::cerr << "服务器启动失败: " << res.second << std::endl;
        return false;
    }
    
    m_server.start();
    m_running = true;
    
    m_sendThread = std::thread(&myWebSocketServer::sendLoop, this);
    m_sendThread.join();
    std::cout << "exit call, exiting... "<< std::endl;
    
    return true;
}

void myWebSocketServer::stop() {
    if (m_running) {
        m_running = false;
        
        m_server.stop();
        
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clients.clear();
        }
        
        std::cout << "服务器已停止" << std::endl;
    }
}

void myWebSocketServer::wait() {
    std::string line;
    std::getline(std::cin, line);
}

void myWebSocketServer::sendToAllClients(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (auto& client : m_clients) {
        if (client && client->getReadyState() == ix::ReadyState::Open) {
            client->send(message);
        }
    }
}

void myWebSocketServer::sendToClient(std::shared_ptr<ix::WebSocket> client, const std::string& message) {
    if (client && client->getReadyState() == ix::ReadyState::Open) {
        client->send(message);
    }
}

void myWebSocketServer::removeClient(std::shared_ptr<ix::WebSocket> client) {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    auto it = std::find(m_clients.begin(), m_clients.end(), client);
    if (it != m_clients.end()) {
        m_clients.erase(it);
        std::cout << "当前连接数: " << m_clients.size() << std::endl;
    }
}

void myWebSocketServer::sendLoop() {
    std::string message;
    std::cout << "服务器: 请输入消息发送(输入'exit'退出): " << std::endl;
    
    while (m_running && std::getline(std::cin, message)) {
        if (message == "exit") {
            stop();
            break;
        }
        
        if (!message.empty()) {
            sendToAllClients("服务器: " + message);
        }
        
        if (m_running) {
            std::cout << "服务器: 请输入消息发送(输入'exit'退出): " << std::endl;
        }
    }
}
    