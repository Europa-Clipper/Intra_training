#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <ixwebsocket/IXWebSocket.h>
#include <string>
#include <atomic>
#include <thread>

class myWebSocketClient {
public:
    myWebSocketClient(const std::string& url);
    ~myWebSocketClient();
    
    void connect();
    void send(const std::string& message);
    inline void startInteractiveMode(){
      m_inputThread = std::thread(&myWebSocketClient::inputLoop, this);
      m_inputThread.join();
    }
    void disconnect();
    
private:
    std::string m_url;
    ix::WebSocket m_webSocket;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_running;
    std::thread m_inputThread;
    
    void inputLoop();
};

#endif // WS_CLIENT_H
    