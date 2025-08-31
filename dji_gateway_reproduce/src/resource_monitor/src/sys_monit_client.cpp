#include <libwebsockets.h>  // 旧版头文件路径（3.x 标准）
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <atomic>

using json = nlohmann::json;
using namespace std;

// 线程安全的运行状态
static atomic<bool> client_running(false);

// 客户端回调函数（旧版 API 兼容）
static int client_callback(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user_data, void *in, size_t len) {
    switch (reason) {
        // 客户端成功连接（旧版宏）
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            cout << "[Client] 已连接到服务器" << endl;
            client_running = true;
            break;

        // 接收服务器数据（旧版宏不变）
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            string data_str((const char *)in, len);
            try {
                json data = json::parse(data_str);
                cout << "\n===== 设备监控数据 =====" << endl;
                cout << "时间戳: " << data["timestamp"] << endl;
                cout << "CPU使用率: " << data["cpu_usage"] << "%" << endl;
                cout << "电池电量: " << data["battery_level"] << "%" << endl;
                cout << "进程数量: " << data["process_count"] << endl;
                cout << "设备名称: " << data["device_name"] << endl;
                cout << "=========================" << endl;
            } catch (exception &e) {
                cerr << "[Client] 数据解析失败: " << e.what() << endl;
            }
            break;
        }

        // 客户端连接错误（旧版宏）
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            cerr << "[Client] 连接失败（检查服务器IP/端口）" << endl;
            client_running = false;
            break;

        // 客户端断开连接（旧版宏不变）
        case LWS_CALLBACK_CLOSED:
            cout << "[Client] 与服务器断开连接" << endl;
            client_running = false;
            break;

        default:
            break;
    }
    return 0;
}

// 客户端协议配置（旧版需显式结束标志）
static const struct lws_protocols client_protocols[] = {
    {
        "dji-monitor-protocol",  // 必须与服务器完全一致（包括大小写）
        client_callback,
        0,
        4096
    },
    {NULL, NULL, 0, 0}
};

// 客户端启动函数
void client_thread(const string &server_ip, int port) {
    struct lws_context_creation_info ctx_info = {0};  // 初始化上下文配置
    struct lws_context *context = nullptr;            // 上下文对象
    struct lws_client_connect_info conn_info = {0};   // 旧版连接配置

    // 配置客户端上下文
    ctx_info.protocols = client_protocols;  // 绑定协议
    ctx_info.options = 0;                   // 旧版无额外选项
    ctx_info.port = CONTEXT_PORT_NO_LISTEN; // 客户端不监听端口（固定值）

    // 创建客户端上下文
    context = lws_create_context(&ctx_info);
    if (!context) {
        cerr << "[Client] 初始化失败（检查 libwebsockets 库）" << endl;
        return;
    }

    // 配置客户端连接信息（旧版核心：用 lws_client_connect_info）
    conn_info.context = context;
    conn_info.address = server_ip.c_str();
    conn_info.port = port;
    conn_info.path = "/";
    conn_info.host = conn_info.address;
    conn_info.origin = "dji-client";
    conn_info.protocol = "dji-monitor-protocol";
    conn_info.ssl_connection = 0;  // 不使用 SSL

    // 发起客户端连接（旧版核心函数）
    struct lws *wsi = lws_client_connect_via_info(&conn_info);
    if (!wsi) {
        cerr << "[Client] 无法创建连接（服务器可能未启动）" << endl;
        lws_context_destroy(context);
        return;
    }

    // 客户端主循环
    cout << "[Client] 正在连接 " << server_ip << ":" << port << "..." << endl;
    while (client_running && lws_service(context, 1000) >= 0) {
        // 1000ms 超时，避免 CPU 占用过高
    }

    // 释放资源
    lws_context_destroy(context);
    cout << "[Client] 客户端已退出" << endl;
}

// 停止客户端（外部接口）
void stop_client() {
    client_running = false;
}