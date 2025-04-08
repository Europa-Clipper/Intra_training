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

  void LogInfo(const char* func, std::string msg);

  std::string get_time_now();

} //namespace dlog
} //namespcae dji

#endif
