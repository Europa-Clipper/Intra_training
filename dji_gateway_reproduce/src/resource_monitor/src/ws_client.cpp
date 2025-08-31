#include "ws_client.h"
#include <iostream>

myWebSocketClient::myWebSocketClient(const std::string& url) 
    : m_url(url), m_connected(false), m_running(false) {
    
    m_webSocket.setUrl(m_url);
    
    // 配置消息回调
    m_webSocket.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                std::cout << "\n收到服务器消息: " << msg->str << std::endl;
                if (m_running) {
                    std::cout << "客户端: 请输入消息(输入'exit'退出): " << std::endl;
                }
            }
            else if (msg->type == ix::WebSocketMessageType::Open) {
                std::cout << "已连接到服务器" << std::endl;
                m_connected = true;
            }
            else if (msg->type == ix::WebSocketMessageType::Close) {
                std::cout << "\n与服务器的连接已关闭" << std::endl;
                m_connected = false;
                m_running = false;
            }
            else if (msg->type == ix::WebSocketMessageType::Error) {
                std::cerr << "\n错误信息: " << msg->errorInfo.reason << std::endl;
                if (msg->errorInfo.http_status != 0) {
                    std::cerr << "HTTP状态码: " << msg->errorInfo.http_status << std::endl;
                }
                m_connected = false;
                m_running = false;
            }
        }
    );
}

myWebSocketClient::~myWebSocketClient() {
    disconnect();
}

void myWebSocketClient::connect() {
    m_running = true;
    m_webSocket.start();
}

void myWebSocketClient::send(const std::string& message) {
    if (m_connected) {
        m_webSocket.send(message);
    } else {
        std::cerr << "未连接到服务器，无法发送消息" << std::endl;
    }
}

// inline void myWebSocketClient::startInteractiveMode() {
//     m_inputThread = std::thread(&myWebSocketClient::inputLoop, this);
//     m_inputThread.join();
// }

void myWebSocketClient::disconnect() {
    m_running = false;
    if (m_connected) {
        m_webSocket.stop();
        m_connected = false;
        
        std::cout << "客户端已退出" << std::endl;
    }
}

void myWebSocketClient::inputLoop() {
    std::string message;
    std::cout << "客户端: 请输入消息(输入'exit'退出): " << std::endl;
    
    while (m_running && std::getline(std::cin, message)) {
        if (message == "exit") {
            disconnect();
            break;
        }
        
        if (!message.empty()) {
            send(message);
        }
        
        if (m_running) {
            std::cout << "客户端: 请输入消息(输入'exit'退出): " << std::endl;
        }
    }
}
    