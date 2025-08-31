#include "../include/sys_monit_server.h"
#include <libwebsockets.h>  // 适配 4.4.0 头文件路径
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <mutex>
#include <cstdlib>
#include <cstring>

using json = nlohmann::json;

// 服务器全局状态（线程安全）
static bool server_running = false;
static struct lws_context *server_context = nullptr;
static std::mutex server_mutex;

// 模拟硬件数据采集
static float get_cpu_usage() {
    static float usage = 10.0f;
    usage += (rand() % 10 > 5) ? 0.5f : -0.5f;
    return std::max(5.0f, std::min(95.0f, usage));
}

static int get_battery_level() {
    static int level = 80;
    level += (rand() % 3 == 0) ? -1 : 0;
    return std::max(10, std::min(100, level));
}

static int get_process_count() {
    return 120 + (rand() % 20);
}

// 服务器回调函数（适配 4.4.0 版本）
static int server_callback(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user_data, void *in, size_t len) {
                            std::cout << "[服务器] 收到 HTTP 握手请求，原始数据: " << std::string((const char*)in, len) << std::endl;
    switch (reason) {
        // 处理 HTTP 握手请求（关键修正）
        case LWS_CALLBACK_HTTP: {
            std::cout << "[服务器] 收到 HTTP 握手请求，开始查找协议..." << std::endl;
            const char *target_proto_name = "dji-monitor-protocol";
            const struct lws_protocols *target_proto = NULL;

            // 遍历协议列表（旧版 API 兼容）
            int proto_index = 0;
            while (true) {
                const struct lws_protocols *proto = lws_protocol_get(wsi);
                if (!proto || !proto->name) {
                    std::cout << "[服务器] 遍历完协议列表，未找到目标协议" << std::endl;
                    break;
                }
                std::cout << "[服务器] 检查协议: " << proto->name << std::endl;
                if (strcmp(proto->name, target_proto_name) == 0) {
                    target_proto = proto;
                    std::cout << "[服务器] 找到目标协议: " << target_proto_name << std::endl;
                    break;
                }
                proto_index++;
            }

            if (!target_proto) {
                std::cerr << "[服务器] 未找到协议：" << target_proto_name << std::endl;
                return -1;
            }

            // 完成握手（传递 5 个参数，适配旧版）
            std::cout << "[服务器] 调用握手函数，完成 WebSocket 升级" << std::endl;
            return lws_callback_http_dummy(wsi, reason, user_data, in, len);
        }

        // 客户端连接建立
        case LWS_CALLBACK_ESTABLISHED:
            std::cout << "[服务器] 客户端已连接，开始推送数据" << std::endl;
            // 触发第一次数据发送
            if (server_running) {
                lws_callback_on_writable(wsi);
            }
            break;
        
        // 发送数据回调
        case LWS_CALLBACK_SERVER_WRITEABLE: {
            // 封装监控数据
            json data;
            data["timestamp"] = time(nullptr);
            data["cpu_usage"] = get_cpu_usage();
            data["battery_level"] = get_battery_level();
            data["process_count"] = get_process_count();
            data["device_name"] = "DJI-B设备";
            std::string json_str = data.dump();

            // 封装 WebSocket 帧（预留头部空间）
            unsigned char buf[LWS_PRE + json_str.size()];
            memcpy(buf + LWS_PRE, json_str.data(), json_str.size());

            // 发送文本帧
            lws_write(wsi, buf + LWS_PRE, json_str.size(), LWS_WRITE_TEXT);
            std::cout << "[服务器] 已发送监控数据，等待下一次发送..." << std::endl;

            // 1秒后再次发送
            if (server_running) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                lws_callback_on_writable(wsi);
            }
            break;
        }

        // 客户端断开连接
        case LWS_CALLBACK_CLOSED:
            std::cout << "[服务器] 客户端已断开连接" << std::endl;
            break;

        default:
            break;
    }
    return 0;
}

// 服务器协议配置（必须以 NULL 结尾）
static const struct lws_protocols server_protocols[] = {
    {
        "dji-monitor-protocol",  // 确保无大小写错误
        server_callback,
        0,
        4096
    },
    {NULL, NULL, 0, 0}
};

// 服务器运行线程
static void server_thread(int port) {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = port;
    info.protocols = server_protocols;

    // 修正：仅保留旧版 4.4.0 支持的基础选项（移除未定义的选项）
    info.options = 0;  // 旧版默认值，不启用额外安全检查

    // 关键：显式设置 uid 和 gid 为 -1（避免权限问题导致握手失败）
    info.uid = -1;
    info.gid = -1;

    // 创建服务器上下文
    server_context = lws_create_context(&info);
    if (!server_context) {
        std::cerr << "[服务器] 无法创建上下文（检查 libwebsockets 库）" << std::endl;
        return;
    }

    std::cout << "[服务器] 已启动，监听端口: " << port << std::endl;

    // 服务器主循环（移除 sleep，确保及时处理事件）
    while (server_running && lws_service(server_context, 0) >= 0) {}

    // 释放资源
    if (server_context) {
        lws_context_destroy(server_context);
        server_context = nullptr;
    }
    std::cout << "[服务器] 已停止" << std::endl;
}

// 启动服务器（外部接口）
void start_monitor_server(int port) {
    std::lock_guard<std::mutex> lock(server_mutex);
    if (!server_running) {
        server_running = true;
        std::thread(&server_thread, port).detach();
    }
}

// 停止服务器（外部接口）
void stop_server() {
    std::lock_guard<std::mutex> lock(server_mutex);
    server_running = false;
}
