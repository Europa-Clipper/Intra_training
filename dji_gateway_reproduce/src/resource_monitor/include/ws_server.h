#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXWebSocket.h>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

namespace ix {
    class WebSocket;
    class ConnectionState;
}

class myWebSocketServer {
public:
    myWebSocketServer(int port);
    ~myWebSocketServer();
    
    bool start();
    void stop();
    void wait();
    
    // 主动发送消息的接口
    void sendToAllClients(const std::string& message);
    void sendToClient(std::shared_ptr<ix::WebSocket> client, const std::string& message);

private:
    int m_port;
    ix::WebSocketServer m_server;
    std::atomic<bool> m_running;
    
    // 客户端连接管理
    std::vector<std::shared_ptr<ix::WebSocket>> m_clients;
    std::mutex m_clientsMutex;
    
    // 服务器发送线程
    std::thread m_sendThread;
    void sendLoop();
    
    // 移除客户端连接
    void removeClient(std::shared_ptr<ix::WebSocket> client);
};

#endif // WS_SERVER_H
    