#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <cstring>
#include <ctime>

namespace fs = std::filesystem;
namespace dji{

namespace gateway{

namespace Shared {
    bool isWSL();

    std::string getCPUModel();

    double getCPUFreq();

    std::string getWSLBattery();

    std::string getLinuxBattery();

    // 获取终端宽度
    int getTerminalWidth();

    // 获取终端高度
    int getTerminalHeight();
    
    // 字符串分割函数
    std::vector<std::string> split(const std::string &s, char delim);
}

// CPU监控命名空间
namespace CPUMonitor {
    struct CPUStats {
        unsigned long long user = 0;
        unsigned long long nice = 0;
        unsigned long long system = 0;
        unsigned long long idle = 0;
        unsigned long long iowait = 0;
        unsigned long long irq = 0;
        unsigned long long softirq = 0;
    };

    std::vector<CPUStats> readCPUStats();

    double calculateUsage(const CPUStats& prev, const CPUStats& curr);

    std::vector<double> getCoreUsages();
}

// 进程监控命名空间 - 改用/proc文件系统实现
namespace ProcMonitor {
    // 进程信息结构体
    struct proc_info {
        pid_t pid;
        pid_t ppid;
        std::string name;
        std::string cmd;
        std::string user;
        char state;
        int p_nice;
        uint64_t mem;
        int threads;
        double cpu_p;
        time_t cpu_s;
        unsigned long long utime;
        unsigned long long stime;
    };

    // 进程状态转换
    std::string get_status(char state);

    // 内存大小格式化
    std::string format_memory(uint64_t bytes);

    // 从/proc/[pid]/stat获取进程基本信息
    bool get_proc_stat(pid_t pid, proc_info& pinfo);

    // 从/proc/[pid]/status获取进程内存信息
    bool get_proc_status(pid_t pid, proc_info& pinfo);

    // 从/proc/[pid]/cmdline获取命令行
    bool get_proc_cmdline(pid_t pid, proc_info& pinfo);

    // 获取进程所属用户名
    bool get_proc_user(pid_t pid, proc_info& pinfo);

    // 计算CPU使用率
    void calculate_cpu_usage(std::vector<proc_info>& processes);

    // 获取所有进程信息
    std::vector<proc_info> get_processes();
    }

    void run_monitor();

  } //gateway
}  //dji