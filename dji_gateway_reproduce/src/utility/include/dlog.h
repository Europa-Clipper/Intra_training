#ifndef DLOG_H
#define DLOG_H

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace dji {
  namespace dlog{

  void LogError(const char* func, std::string msg);

  void LogWarn(const char* func, std::string msg);

  template<typename T, typename... Args>
  void LogInfo(T first, Args... args);

  void log();

  template<typename T, typename... Args>
  void log(T first, Args... args);

  std::string get_time_now();

  template<typename T, typename... Args>
    void log(T first, Args... args) {
      std::cout << first;
      if constexpr (sizeof...(args) > 0) {
        std::cout << " ";
      }
      log(args...);
    }

    template<typename T, typename... Args>
    void LogInfo(T first, Args... args){
      auto now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      std::tm* localTime = std::localtime(&currentTime);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
      std::cout <<"[" <<std::put_time(localTime, "%Y-%m-%d %H:%M:%S")<< '.' << std::setfill('0') << std::setw(3) 
      << ms.count()<<"][I]["<<first<<"]"<<"Info:";
      if constexpr (sizeof...(args) > 0) {
        std::cout << " ";
      }
      log(args...);
      
      std::cout.flush();
    }

    template<typename T, typename... Args>
    void LogError(T first, Args... args){
      auto now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      std::tm* localTime = std::localtime(&currentTime);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
      std::cerr <<"[" <<std::put_time(localTime, "%Y-%m-%d %H:%M:%S")<< '.' << std::setfill('0') 
      << std::setw(3) << ms.count()<< "][E]["<< first <<"]"<< "Error:";
      if constexpr (sizeof...(args) > 0){
        std::cerr << " ";
      }
      log(args...);

      std::exit(1);
    }

    template<typename T, typename... Args>
    void LogWarn(T first, Args... args){
      auto now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      std::tm* localTime = std::localtime(&currentTime);
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
      std::cerr <<"[" <<std::put_time(localTime, "%Y-%m-%d %H:%M:%S")<< '.' << std::setfill('0') 
      << std::setw(3) << ms.count()<< "][W]["<< first <<"]"<< "Warn:";
      if constexpr (sizeof...(args) > 0){
        std::cerr << " ";
      }
      log(args...);
    }



} //namespace dlog
} //namespcae dji

#endif
