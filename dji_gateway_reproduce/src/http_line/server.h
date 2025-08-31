#ifndef SERVER_H
#define SERVER_H
#include <fstream>
#include <iostream>
#include <chrono>
#include <queue>

#include "../../3rdParty/cpp-httplib-master/httplib.h"
#include "../utility/include/dlog.h"

namespace dji{
  namespace gateway{
    
    void printRequestInfo(const httplib::Request& req);

    // void printRequestInfo(const httplib::Request& req) {
    //   dlog::LogInfo(__func__, "printing request info...");

    //   std::cout << "Request path: " << req.path << std::endl;

    //   if (!req.params.empty()) {
    //     std::cout << "Query Parameters:" << std::endl;
    //     for (const auto& param : req.params) {
    //       std::cout << "  " << param.first << ": " << param.second << std::endl;
    //     }
    //   }

    //   // std::cout << "Request Headers:" << std::endl;
    //   // for (const auto& header : req.headers) {
    //   //   std::cout << "  " << header.first << ": " << header.second << std::endl;
    //   // }

    //   if (!req.body.empty()) {
    //     std::cout<<"————————————————————————————————————"<<std::endl<<std::endl;
    //     //std::cout << "bod:" << std::endl;
    //     std::cout << req.body << std::endl;
    //     std::cout<<"————————————————————————————————————"<<std::endl<<std::endl;
    //   }
    // }

    struct chatmsg{
      size_t id;
      size_t curnum;
      std::string cnt;
      std::string timestamp;
    };

    struct chatmsg string_to_chatmsg(const std::string& str);

  } // namespace gateway
  
}

#endif