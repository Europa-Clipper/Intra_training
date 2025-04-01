#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace dji {
  namespace dlog{

  void LogError(const char* func, std::string msg);

  void LogWarn(const char* func, std::string msg);

  void LogInfo(const char* func, std::string msg);

} //namespace dlog
} //namespcae dji


