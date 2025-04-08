#include <fstream>
#include <iostream>
#include <chrono>

#include "../../3rdParty/cpp-httplib-master/httplib.h"
#include "../utility/include/dlog.h"



void printRequestInfo(const httplib::Request& req) {
  std::cout << "Request Method: " << req.method << std::endl;

  std::cout << "Request Path: " << req.path << std::endl;

  if (!req.params.empty()) {
      std::cout << "Query Parameters:" << std::endl;
      for (const auto& param : req.params) {
          std::cout << "  " << param.first << ": " << param.second << std::endl;
      }
  }

  std::cout << "Request Headers:" << std::endl;
  for (const auto& header : req.headers) {
      std::cout << "  " << header.first << ": " << header.second << std::endl;
  }

  if (!req.body.empty()) {
      std::cout << "Request Body:" << std::endl;
      std::cout << req.body << std::endl;
  }
}


int main() {
    httplib::Server svr;
    using namespace dji;
    
    svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {
        if (req.has_header("Content-Type") && req.get_header_value("Content-Type") == "text/plain") {
          
          const char* dirname = "received_files";
          int status = mkdir(dirname, 0777); // 使用 mkdir 函数创建目录，权限设置为 0777（可读、可写、可执行）
          if (status == 0) {
              dlog::LogInfo(__func__, "Directory created successfully.");
          } else {
              dlog::LogWarn(__func__, "Failed to create directory");
          }

          //std::string time = dji::dlog::get_time_now();
          //std::string store_path = std::string(dirname) + "/" + time;
          std::ofstream outFile(std::string(dirname) + "/" + dji::dlog::get_time_now());
          //dlog::LogInfo(__func__, "header: ");
          printRequestInfo(req);
            if (outFile) {
                outFile << req.body;
                outFile.close();
                res.set_content("File received success", "text/plain");
            } else {
                res.status = 500;
                res.set_content("Failed to save file", "text/plain");
            }
        } else {
            res.status = 400;
            res.set_content("Invalid content type", "text/plain");
        }
    });

    if (svr.listen("172.21.204.118", 9191)) {
        dlog::LogInfo("server", "Server is running on port 9191...");
    } else {
        dlog::LogWarn("server", "Failed to start server.");
    }

    return 0;
  
}