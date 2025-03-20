#include <iostream>
#include <cstdint>
#include <chrono>
#include <stdint.h>
#include <iomanip>
#include <x86intrin.h>
#include <unistd.h>
#include <fstream>
#include <vector>


unsigned long long get_cpu_frequency() {
    unsigned long long freq = sysconf(_SC_CLK_TCK);
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz")!= std::string::npos) {
            size_t pos = line.find(":");
            if (pos!= std::string::npos) {
                std::string freq_str = line.substr(pos + 2);
                try {
                    freq = std::stoull(freq_str) * 1000000; // 转换为 Hz
                } catch (const std::exception& e) {
                    std::cerr << "Error converting CPU frequency: " << e.what() << std::endl;
                }
                break;
            }
        }
    }
    return freq;
}


int main() {
    std::vector<int>vec(10, 0);
    unsigned long long freq = get_cpu_frequency();
    std::cout << "freq = " << freq << std::endl;

    std::cout << "单次io时长测试 :" << std::endl 
    << "--------------------------------------------------------------" << std:: endl;
    uint64_t iostart = __rdtsc();
    std::cout << "kaishiioceshiboviwvboiwe894545fedvefbecvefbefve ev4tb4 yf298yv82hd29vb2ehd2g3gv92ehc98hrv83j9efvnueirbvuncieubvumcw0cjivbwincwb9uiwv][];.,';eck0wej47fhduvb73hnckn"
        << std::endl;
    uint64_t ioend = __rdtsc();
    uint64_t iotime = ioend - iostart;
    std:: cout << "io elapsed cycle = " << iotime << " = " << (iotime*1e9/freq) << "纳秒" << std::endl;
 

    std::cout << "rdtsc : " << std::endl << "--------------------------------------------------" << std::endl;
    unsigned long long begin = __rdtsc();
    for(unsigned long long i = 0; i < 100; ++i){
        // if(i == 50){
        //     unsigned long long endd = __rdtsc();
        //     unsigned long long elapsed_cycle = endd - begin;
        //     unsigned long long transtime = (elapsed_cycle * 1e9/ freq);
        //     std::cout << "elapsed cycled: " << elapsed_cycle << std::endl;
        //     std::cout << "elapsed cycle trans to nano: " << transtime << "纳秒" << std:: endl;
        // }


        vec[i % 10] = 20;
    }
    unsigned long long endd = __rdtsc();
    unsigned long long elapsed_cycle = endd - begin;
    unsigned long long transtime = (elapsed_cycle * 1e9/ freq);
    std::cout << "完整操作循环： " << elapsed_cycle << std:: endl;
    std::cout << "elapsed cycle trans to nano: " << transtime << "纳秒 = " << transtime/1e9 << "秒" << std:: endl;



    std::cout << "chrono :" << std::endl << "------------------------------------------------" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    //unsigned long long begin = __rdtsc();
    for (unsigned long long i = 0; i < 100; ++i) {
        // if(i == 50){
        //     std::cout << "i == 50" << std:: endl;
        //     auto midend = std::chrono::high_resolution_clock::now();
        //     auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(midend - start);
        //     std::cout << "已耗时：" << duration.count() << " 纳秒" << std::endl;
        // }


        vec[i % 10] = 30;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    auto duration1 = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "完整操作耗时: " << duration.count() << " 纳秒 = " << duration.count()/1e9 << "秒" << std::endl;


    std::time_t start_time_t = std::chrono::high_resolution_clock::to_time_t(start);
    std::time_t end_time_t = std::chrono::high_resolution_clock::to_time_t(end);
    std::cout << "开始时间: " << std::put_time(std::localtime(&start_time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "结束时间: " << std::put_time(std::localtime(&end_time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
    while(1){}
    
    return 0;
}