#ifndef SYS_MONIT_CLIENT_H
#define SYS_MONIT_CLIENT_H

#include <string>

// 启动客户端
void start_monitor_client(std::string &server_ip, int port);
void client_thread(const std::string& server_ip, int port);

// 停止客户端（供信号处理调用）
void stop_client();

#endif // WEBSOCKET_CLIENT_H
    