#ifndef SYS_MONIT_SERVER_H
#define SYS_MONIT_SERVER_H

#include <string>

// 启动服务器
void start_monitor_server(int port);

// 停止服务器（供信号处理调用）
void stop_server();

#endif // WEBSOCKET_SERVER_H
    