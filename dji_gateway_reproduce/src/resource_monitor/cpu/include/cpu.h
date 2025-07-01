#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <deque>

namespace fs = std::filesystem;

namespace dji{
  namespace gateway{
    namespace cpu {
      std::vector<long long> core_old_totals;
      std::vector<long long> core_old_idles;
      std::vector<std::string> available_fields = {"Auto", "total"};
      std::vector<std::string> available_sensors = {"Auto"};
      cpu_info current_cpu;
      fs::path freq_path = "/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq";
      bool got_sensors{};
      bool cpu_temp_only{};

      //* Populate found_sensors map
      bool get_sensors();

      //* Get current cpu clock speed
      std::string get_cpuHz();

      //* Search /proc/cpuinfo for a cpu name
      std::string get_cpuName();

      struct Sensor {
        fs::path path;
        std::string label;
        int64_t temp{};
        int64_t high{};
        int64_t crit{};
      };

      struct cpu_info {
        std::unordered_map<std::string, std::deque<long long>> cpu_percent = {
          {"total", {}},
          {"user", {}},
          {"nice", {}},
          {"system", {}},
          {"idle", {}},
          {"iowait", {}},
          {"irq", {}},
          {"softirq", {}},
          {"steal", {}},
          {"guest", {}},
          {"guest_nice", {}}
        };
        std::vector<std::deque<long long>> core_percent;
        std::vector<std::deque<long long>> temp;
        long long temp_max = 0;
        std::array<double, 3> load_avg;
      };

      std::unordered_map<std::string, Sensor> found_sensors;
      std::string cpu_sensor;
      std::vector<std::string> core_sensors;
      std::unordered_map<int, int> core_mapping;
      
    }  //cpu
  }  //gateway
} //dji

