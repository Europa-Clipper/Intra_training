#include <sstream>

#include "../include/dlog.h"

namespace dji{
  namespace dlog{

    void LogError(const char* func, std::string msg){
      auto now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      std::tm* localTime = std::localtime(&currentTime);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
      std::cerr <<"[" <<std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
      std::cerr << '.' << std::setfill('0') << std::setw(3) << ms.count()<<"][E]"<<"["<<func<<"]"<<"Error: "<<msg<<std::endl;

      std::exit(1);
    }

    void LogWarn(const char* func, std::string msg){
      auto now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      std::tm* localTime = std::localtime(&currentTime);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
      std::cout <<"[" <<std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
      std::cout << '.' << std::setfill('0') << std::setw(3) << ms.count()<<"][W]"<<"["<<func<<"]"<<"Warn: "<<msg<<std::endl;
      std::cerr.flush();
    }

    void LogInfo(const char* func, std::string msg){
      auto now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      std::tm* localTime = std::localtime(&currentTime);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
      std::cout <<"[" <<std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
      std::cout << '.' << std::setfill('0') << std::setw(3) << ms.count()<<"][I]"<<"["<<func<<"]"<<"Info: "<<msg<<std::endl;
      std::cout.flush();
    }

    std::string get_time_now(){
      auto now = std::chrono::system_clock::now();
      auto in_time_t = std::chrono::system_clock::to_time_t(now);
      
      std::stringstream ss;
      ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
      return ss.str();
    }

} //namespace dlog
} //namespace dji