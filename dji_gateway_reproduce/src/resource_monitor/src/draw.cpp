#include "../include/draw.h"

namespace fs = std::filesystem;
namespace dji{

namespace gateway{

namespace Shared {
    bool isWSL() {
        std::ifstream procVersion("/proc/version");
        if (!procVersion.is_open()) return false;

        std::string line;
        std::getline(procVersion, line);
        return line.find("Microsoft") != std::string::npos || line.find("WSL") != std::string::npos;
    }

    std::string getCPUModel() {
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (!cpuinfo.is_open()) return "Unknown";

        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("model name") != std::string::npos) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    return line.substr(colon + 2);
                }
            }
        }
        return "Unknown";
    }

    double getCPUFreq() {
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (!cpuinfo.is_open()) return 0.0;

        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("cpu MHz") != std::string::npos) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    return std::stod(line.substr(colon + 2));
                }
            }
        }
        return 0.0;
    }

    std::string getWSLBattery() {
        std::string cmd = "powershell.exe \"Get-WmiObject -Class Win32_Battery | Select-Object EstimatedChargeRemaining, BatteryStatus | ConvertTo-Json\"";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "Error: WSL Battery Query Failed";

        std::string result;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);

        size_t chargePos = result.find("EstimatedChargeRemaining");
        size_t statusPos = result.find("BatteryStatus");
        if (chargePos == std::string::npos || statusPos == std::string::npos) {
            return "Battery: Unknown (WSL)";
        }

        chargePos = result.find(":", chargePos) + 2;
        size_t chargeEnd = result.find(",", chargePos);
        std::string chargeStr = result.substr(chargePos, chargeEnd - chargePos);

        statusPos = result.find(":", statusPos) + 2;
        size_t statusEnd = result.find("}", statusPos);
        std::string statusStr = result.substr(statusPos, statusEnd - statusPos);

        return "Battery: " + chargeStr + "% (" + (statusStr == "1" ? "Charging" : "Discharging") + ")";
    }

    std::string getLinuxBattery() {
        std::ifstream capacity("/sys/class/power_supply/BAT0/capacity");
        std::ifstream status("/sys/class/power_supply/BAT0/status");
        if (!capacity.is_open() || !status.is_open()) {
            return "Battery: Not Detected";
        }

        std::string capStr, statusStr;
        std::getline(capacity, capStr);
        std::getline(status, statusStr);
        return "Battery: " + capStr + "% (" + statusStr + ")";
    }

    int getTerminalWidth() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
    }

    int getTerminalHeight() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
    }
    
    std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delim)) {
            tokens.push_back(token);
        }
        return tokens;
    }
}

namespace CPUMonitor {
    // struct CPUStats {
    //     unsigned long long user = 0;
    //     unsigned long long nice = 0;
    //     unsigned long long system = 0;
    //     unsigned long long idle = 0;
    //     unsigned long long iowait = 0;
    //     unsigned long long irq = 0;
    //     unsigned long long softirq = 0;
    // };

    std::vector<CPUStats> readCPUStats() {
        std::vector<CPUStats> stats;
        std::ifstream statFile("/proc/stat");
        if (!statFile.is_open()) return stats;

        std::string line;
        while (std::getline(statFile, line)) {
            if (line.substr(0, 3) != "cpu") continue;

            std::istringstream iss(line);
            std::string cpuName;
            iss >> cpuName;

            CPUStats s;
            iss >> s.user >> s.nice >> s.system >> s.idle >> s.iowait 
                >> s.irq >> s.softirq;
            stats.push_back(s);
        }
        return stats;
    }

    double calculateUsage(const CPUStats& prev, const CPUStats& curr) {
        unsigned long long totalPrev = prev.user + prev.nice + prev.system + prev.idle + 
                                      prev.iowait + prev.irq + prev.softirq;
        unsigned long long totalCurr = curr.user + curr.nice + curr.system + curr.idle + 
                                      curr.iowait + curr.irq + curr.softirq;
        unsigned long long diffTotal = totalCurr - totalPrev;

        unsigned long long diffIdle = curr.idle - prev.idle;
        unsigned long long diffUsed = diffTotal - diffIdle;

        if (diffTotal == 0) return 0.0;
        return (double)diffUsed / diffTotal * 100.0;
    }

    std::vector<double> getCoreUsages() {
        static std::vector<CPUStats> prevStats;
        auto currStats = readCPUStats();

        std::vector<double> usages;
        if (prevStats.empty()) {
            usages.resize(currStats.size() - 1, 0.0);
        } else {
            for (size_t i = 1; i < currStats.size(); ++i) {
                double usage = calculateUsage(prevStats[i], currStats[i]);
                usages.push_back(usage);
            }
        }

        prevStats = currStats;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return usages;
    }
}

namespace ProcMonitor {
    // struct proc_info {
    //     pid_t pid;
    //     pid_t ppid;
    //     std::string name;
    //     std::string cmd;
    //     std::string user;
    //     char state;
    //     int p_nice;
    //     uint64_t mem;
    //     int threads;
    //     double cpu_p;
    //     time_t cpu_s;
    //     unsigned long long utime;
    //     unsigned long long stime;
    // };


    std::string get_status(char state) {
        switch (state) {
            case 'R': return "Running";
            case 'S': return "Sleeping";
            case 'D': return "Disk Sleep";
            case 'Z': return "Zombie";
            case 'T': return "Stopped";
            case 't': return "Tracing Stop";
            case 'X': return "Dead";
            case 'I': return "Idle";
            default: return "Unknown";
        }
    }

    std::string format_memory(uint64_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_idx = 0;
        double size = bytes;
        while (size >= 1024 && unit_idx < 4) {
            size /= 1024;
            unit_idx++;
        }
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f %s", size, units[unit_idx]);
        return buf;
    }

    bool get_proc_stat(pid_t pid, proc_info& pinfo) {
        std::string path = "/proc/" + std::to_string(pid) + "/stat";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        std::getline(file, line);
        std::vector<std::string> parts = Shared::split(line, ' ');
        
        if (parts.size() < 24) return false;
        
        pinfo.pid = pid;
        pinfo.ppid = std::stoi(parts[3]);
        pinfo.state = parts[2][0];
        pinfo.utime = std::stoull(parts[13]);
        pinfo.stime = std::stoull(parts[14]);
        pinfo.p_nice = std::stoi(parts[18]);
        pinfo.threads = std::stoi(parts[19]);
        pinfo.cpu_s = std::stoull(parts[21]) / sysconf(_SC_CLK_TCK);
        
        return true;
    }

    bool get_proc_status(pid_t pid, proc_info& pinfo) {
        std::string path = "/proc/" + std::to_string(pid) + "/status";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.substr(0, 6) == "VmRSS:") {
                std::istringstream iss(line);
                std::string label;
                unsigned long rss;
                std::string unit;
                iss >> label >> rss >> unit;
                
                if (unit == "kB") {
                    pinfo.mem = rss * 1024;
                } else {
                    pinfo.mem = rss;
                }
                return true;
            }
        }
        return false;
    }

    bool get_proc_cmdline(pid_t pid, proc_info& pinfo) {
        std::string path = "/proc/" + std::to_string(pid) + "/cmdline";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        std::getline(file, line);
        
        std::replace(line.begin(), line.end(), '\0', ' ');
        if (!line.empty() && line.back() == ' ') {
            line.pop_back();
        }
        
        pinfo.cmd = line;
        if (pinfo.cmd.empty()) {
            path = "/proc/" + std::to_string(pid) + "/stat";
            std::ifstream stat_file(path);
            if (stat_file.is_open()) {
                std::string stat_line;
                std::getline(stat_file, stat_line);
                size_t start = stat_line.find('(') + 1;
                size_t end = stat_line.find(')');
                if (start != std::string::npos && end != std::string::npos) {
                    pinfo.name = stat_line.substr(start, end - start);
                    pinfo.cmd = pinfo.name;
                }
            }
        } else {
            pinfo.name = pinfo.cmd.substr(0, pinfo.cmd.find(' '));
            size_t last_slash = pinfo.name.find_last_of('/');
            if (last_slash != std::string::npos) {
                pinfo.name = pinfo.name.substr(last_slash + 1);
            }
        }
        
        return true;
    }

    bool get_proc_user(pid_t pid, proc_info& pinfo) {
        std::string path = "/proc/" + std::to_string(pid) + "/status";
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.substr(0, 4) == "Uid:") {
                std::istringstream iss(line);
                std::string label;
                uid_t uid;
                iss >> label >> uid;
                
                struct passwd* pwd = getpwuid(uid);
                if (pwd) {
                    pinfo.user = pwd->pw_name;
                } else {
                    pinfo.user = std::to_string(uid);
                }
                return true;
            }
        }
        return false;
    }

    void calculate_cpu_usage(std::vector<proc_info>& processes) {
        static std::unordered_map<pid_t, std::pair<unsigned long long, unsigned long long>> prev_cpu;
        static unsigned long long prev_total = 0;
        
        std::ifstream stat_file("/proc/stat");
        std::string line;
        std::getline(stat_file, line);
        std::vector<std::string> parts = Shared::split(line, ' ');
        
        unsigned long long total = 0;
        for (size_t i = 1; i < parts.size(); ++i) {
            if (!parts[i].empty()) {
                total += std::stoull(parts[i]);
            }
        }
        
        unsigned long long diff_total = total - prev_total;
        
        for (auto& proc : processes) {
            auto it = prev_cpu.find(proc.pid);
            if (it != prev_cpu.end()) {
                unsigned long long prev_utime = it->second.first;
                unsigned long long prev_stime = it->second.second;
                
                unsigned long long diff_utime = proc.utime - prev_utime;
                unsigned long long diff_stime = proc.stime - prev_stime;
                unsigned long long diff_used = diff_utime + diff_stime;
                
                if (diff_total > 0) {
                    proc.cpu_p = (double)diff_used / diff_total * 100.0 * sysconf(_SC_NPROCESSORS_CONF);
                } else {
                    proc.cpu_p = 0.0;
                }
            } else {
                proc.cpu_p = 0.0;
            }
            
            prev_cpu[proc.pid] = {proc.utime, proc.stime};
        }
        
        std::vector<pid_t> to_remove;
        for (const auto& entry : prev_cpu) {
            bool found = false;
            for (const auto& proc : processes) {
                if (proc.pid == entry.first) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                to_remove.push_back(entry.first);
            }
        }
        for (pid_t pid : to_remove) {
            prev_cpu.erase(pid);
        }
        
        prev_total = total;
    }

    std::vector<proc_info> get_processes() {
        std::vector<proc_info> processes;
        DIR* dir = opendir("/proc");
        if (!dir) return processes;

        dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR) {
                char* endptr;
                pid_t pid = (pid_t)strtol(entry->d_name, &endptr, 10);
                if (*endptr == '\0' && pid > 0) {
                    proc_info pinfo;
                    if (get_proc_stat(pid, pinfo) && 
                        get_proc_status(pid, pinfo) && 
                        get_proc_cmdline(pid, pinfo) && 
                        get_proc_user(pid, pinfo)) {
                        processes.push_back(pinfo);
                    }
                }
            }
        }
        closedir(dir);

        calculate_cpu_usage(processes);

        std::sort(processes.begin(), processes.end(), 
                 [](const proc_info& a, const proc_info& b) {
                     return a.cpu_p > b.cpu_p;
                 });

        return processes;
    }
}

void run_monitor() {
    bool isWSL = Shared::isWSL();
    std::string cpuModel = Shared::getCPUModel();
    std::string batteryInfo = isWSL ? Shared::getWSLBattery() : Shared::getLinuxBattery();
    unsigned int coreCount = std::thread::hardware_concurrency();

    std::cout << "              =====================================================================    DJI 车载诊断系统    =====================================================================      " << std::endl;
    std::cout << "vir: " << (isWSL ? "WSL" : "Linux") << std::endl;
    std::cout << "CPU model: " << cpuModel << std::endl;
    std::cout << "core num: " << coreCount << std::endl;
    std::cout << batteryInfo << std::endl;
    
    int coreUsageLine = 6;
    int processHeaderLine = coreUsageLine + (coreCount + 3) / 4 + -2; 
    
    while (true) {
        auto coreUsages = CPUMonitor::getCoreUsages();
        int terminalWidth = Shared::getTerminalWidth();
        int coreInfoWidth = 13;
        int coresPerLine = terminalWidth / coreInfoWidth;
        if (coresPerLine < 1) coresPerLine = 1;


        std::cout << "\033[" << coreUsageLine << ";0H";
        std::cout << "cpu coreusage:" << std::endl;
        
        for (size_t i = 0; i < coreUsages.size(); ++i) {
            if (i % coresPerLine == 0) {
                std::cout << "\033[" << (coreUsageLine + 1 + i / coresPerLine) << ";0H";
                std::cout << std::string(terminalWidth, ' ');
                std::cout << "\033[" << (coreUsageLine + 1 + i / coresPerLine) << ";0H";
            }
            std::cout << "core " << std::setw(2) << i << ": " 
                      << std::fixed << std::setprecision(1) 
                      << std::setw(5) << coreUsages[i] << "% ";
        }

        auto processes = ProcMonitor::get_processes();
        int terminalHeight = Shared::getTerminalHeight();
        int maxProcesses = terminalHeight - processHeaderLine - 2; 
        if (maxProcesses < 1) maxProcesses = 10;

        std::cout << "\033[" << processHeaderLine << ";0H";
        std::cout << std::string(terminalWidth, ' ');
        std::cout << "\033[" << processHeaderLine << ";0H";
        std::cout << "proc info (tol " << processes.size() << " , cur " << std::min(maxProcesses, (int)processes.size()) << " ):" << std::endl;
        
        std::cout << "\033[" << processHeaderLine + 1 << ";0H";
        std::cout << std::left 
                  << std::setw(6) << "PID" 
                  << std::setw(7) << "PPID" 
                  << std::setw(8) << "status" 
                  << std::setw(12) << "proc name" 
                  << std::setw(10) << "CPU(%)" 
                  << std::setw(10) << "mem" 
                  << std::setw(12) << "user" 
                  << "ord" << std::endl;

        for (int i = 0; i < std::min(maxProcesses, (int)processes.size()); ++i) {
            const auto& proc = processes[i];
            std::cout << "\033[" << processHeaderLine + 2 + i << ";0H";
            std::cout << std::left 
                      << std::setw(6) << proc.pid 
                      << std::setw(6) << proc.ppid 
                      << std::setw(10) << ProcMonitor::get_status(proc.state) 
                      << std::setw(12) << (proc.name.size() > 12 ? proc.name.substr(0, 12) + "..." : proc.name) 
                      << std::setw(5) << std::fixed << std::setprecision(1) << proc.cpu_p 
                      << std::setw(10) << ProcMonitor::format_memory(proc.mem) 
                      << std::setw(13) << proc.user;
            

            std::string cmd = proc.cmd;
            int remainingWidth = terminalWidth - 6 - 6 - 10 - 15 - 12 - 12 - 10;
            if (remainingWidth > 0 && cmd.size() > (size_t)remainingWidth) {
                cmd = cmd.substr(0, remainingWidth - 3) + "...";
            }
            std::cout << cmd << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}


}// gateway

}//dji

int main() {
    std::cout << "\033[?1049h";
    std::cout << "\033[H\033[J";

    try {
        dji::gateway::run_monitor();
    } catch (const std::exception& e) {
        std::cerr << "moniter error: " << e.what() << std::endl;
        std::cout << "\033[?1049l";
        return 1;
    }

    std::cout << "\033[?1049l";
    return 0;
}