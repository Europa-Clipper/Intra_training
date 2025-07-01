#include <fstream>
#include <iostream>
#include <chrono>
#include <queue>

#include "../../3rdParty/cpp-httplib-master/httplib.h"
#include "../utility/include/dlog.h"


enum path{
  upload,
  getFilesInfo,
  null,
  getFiles,
  jobDone,
};

void printRequestInfo(const httplib::Request& req) {
  dji::dlog::LogInfo(__func__, "printing request info...");
  //std::cout << "Request Method: " << req.method << std::endl;

  std::cout << "Request path: " << req.path << std::endl;

  if (!req.params.empty()) {
    std::cout << "Query Parameters:" << std::endl;
    for (const auto& param : req.params) {
      std::cout << "  " << param.first << ": " << param.second << std::endl;
    }
  }

  // std::cout << "Request Headers:" << std::endl;
  // for (const auto& header : req.headers) {
  //   std::cout << "  " << header.first << ": " << header.second << std::endl;
  // }

  if (!req.body.empty()) {
    std::cout << "bod:" << std::endl;
    std::cout << req.body << std::endl;
  }
}

struct chatmsg{
  size_t id;
  size_t curnum;
  std::string cnt;
  std::string timestamp;
};

struct chatmsg string_to_chatmsg(const std::string& str){
  struct chatmsg msg;//"id=7curnum=201cnt=123time=2025.4.16 15:57"
  msg.id = std::stoi(str.substr(str.find("id=")+3, str.find("id=")+4));
  msg.curnum = std::stoi(str.substr(str.find("curnum=") + 7, str.find("cnt")-1 - str.find("curnum")-6));
  msg.cnt = (str.substr(str.find("cnt=")+4, str.find("time=")-1 - str.find("cnt=")-3));
  msg.timestamp = str.substr(str.find("time=")+5);
  std::cout<<"id="<<msg.id<<"  curnum="<<msg.curnum<<"  cnt="<<msg.cnt<<"  time="<<msg.timestamp<<std::endl;
  return msg;
}


// void serverThread(std::queue<std::string>& job){
//   httplib::Server svr;
//   using namespace dji;

//   std::string buffer;
//   std::string store_path{"/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/store_path"};

//   svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {

//       res.set_header("Access-Control-Allow-Origin", "*");

//       if (req.has_header("Content-Type") && req.get_header_value("Content-Type") == "text/plain") {

//         const char* dirname = "received_files";
//         int status = mkdir(dirname, 0777); // 使用 mkdir 函数创建目录，权限设置为 0777（可读、可写、可执行)
//         if (status == 0) {
//           dlog::LogInfo(__func__, "Directory created successfully.");
//         } else {
//           dlog::LogWarn(__func__, "Failed to create directory");
//         }

//         std::ofstream outFile(std::string(dirname) + "/" + dlog::get_time_now());
//         //dlog::LogInfo(__func__, "header: ");
//         printRequestInfo(req);
//           if (outFile) {
//               outFile << req.body;
//               outFile.close();
//               res.set_content("kan dao zhe ge ,shuo ming ce shi cheng gong le", "text/plain");
//           } else {
//               res.status = 500;
//               res.set_content("Failed to save file", "text/plain");
//           }
//         } else {
//           res.status = 400;
//           res.set_content("Invalid content type", "text/plain");
//         }
//     });

//   svr.Post("/getJob", [&job](const httplib::Request& req, httplib::Response& res){
//     res.set_header("Acces-Control-Allow-Orign", "*");

//     assert(req.path == "/getJob");
//     if(job.size() != 0){
//       res.set_content(job.front(), "text/plain");
//       dlog::LogInfo(__func__, "job sent: ", job.front());
//       job.pop();
//       dlog::LogInfo(__func__, "receive getJob request and sent");
//     }
//     else{
//       dlog::LogWarn(__func__, "no current job !");
//       res.set_content("null", "text/plain");
//     }
//   });

//   svr.Post("/sendFilesInfo", [](const httplib::Request& req, httplib::Response& res){
//     res.set_header("Acces-Control-Allow-Orign", "*");
//     printRequestInfo(req);
//     res.set_content("shou dao files info le", "text/plain");
    
//   });

//   svr.Post("/sendFile", [&buffer](const httplib::Request& req, httplib::Response& res){
//     dlog::LogInfo(__func__, "receiving ...");
//     buffer += req.body;
//     res.set_content("part get", "text/plain");
//   });

//   svr.Post("/fileDone", [&buffer, store_path](const httplib::Request& req, httplib::Response& res){
//     dlog::LogInfo(__func__, " end tag");
//     std::ofstream file(store_path + "/" + req.body);
//     dlog::LogInfo(__func__, "REoF:" , req.body);
//     if(file.is_open()){
//       file << buffer ;
//       file.close();
//     }
//     else{
//       dji::dlog::LogWarn(__func__, "file not open");
//     }
//     res.set_content("get", "text/plain");
//     buffer.clear();
//   });

//   if (svr.listen("0.0.0.0", 8080)) {
//   //if (svr.listen("localhost", 8080)) {
//       dlog::LogInfo("server", "Server is running on port 9191...");
//     } else {
//       dlog::LogWarn("server", "Failed to start server.");
//     }
// }

int main() {
  std::queue<std::string>job;
  std::string cmd;
  bool end_flag{false};

  while(1){
    httplib::Server svr;
    std::cout<<"============================================================================================================"<<std::endl;
    std::cout<<"current job:"<<std::endl;
    if(job.size() == 0){
      std::cout<<"null !"<<std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout <<"||   get files info   ||   press 1"<<std::endl
                <<"||     get files      ||   press 2"<<std::endl
                <<"||       chat         ||   press 3"<<std::endl
                <<"||    chat server     ||   press 4"<<std::endl;

      std::getline(std::cin, cmd);

      if(cmd == "1"){
        job.emplace("getFilesInfo");
      }
      else if(cmd == "2"){
        job.emplace("getFiles");
      }
      else if(cmd == "3"){

        std::unique_ptr<std::thread> chatThread = std::make_unique<std::thread>([&svr](){
          bool getrecv{false};
          std::string content{};
          
          svr.Post("/chat", [&content](const httplib::Request& req, httplib::Response& res){
            std::cout<<dji::dlog::get_time_now()<<"  "<<req.body<<std::endl<<"type retrun:"<<std::endl;
            std::getline(std::cin, content);
            std::cout<<"content is :"<<content<<std::endl;
            // int ackbc = stoi(req.body.substr(req.body.find("ack=")));
            res.set_content(content, "text/plain");
          });

          svr.listen("localhost", 8080);
          //svr.listen("172.21.204.118", 8080);
        });
        if(chatThread->joinable()){
          chatThread->join();
        }
      }
      else if(cmd == "4"){
        std::unique_ptr<std::thread> chatServerThread = std::make_unique<std::thread>([&svr](){
          int id_count{0};
          std::vector<chatmsg> msgline{};
          svr.Post("/serverchat", [&msgline](const httplib::Request& req, httplib::Response& res){
            //req.body is incoming chatmsg...
            msgline.emplace_back(string_to_chatmsg(req.body)/*body to chatmsg*/);


          });

          svr.Post("/startchat", [&id_count, &msgline](const httplib::Request& req, httplib::Response& res){
            res.set_content("id=" + std::to_string(id_count) + "//curnum=" + std::to_string(msgline.size()), "text/plain"); //ex: "id=2//curnum=30"
            ++id_count;
          });
          std::string strr{"id=7curnum=201cnt=zheshiyigeceshi,,kankanshifouzhengquetime=2025.4.16 15:57"};
          std::cout<<"strr="<<strr<<std::endl;
          string_to_chatmsg(strr);
        });
        
        chatServerThread->join();
      
      }
      else{
        std::cout<<"valid input"<<std::endl;
      }
      continue;
    }
    else{
      std::cout<<job.front()<<std::endl;
      std::unique_ptr<std::thread> unit1 = std::make_unique<std::thread>([&job, &svr, &end_flag](){
        using namespace dji;

        
        std::string buffer;
        std::string store_path{"/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/store_path"};

        svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {

            res.set_header("Access-Control-Allow-Origin", "*");

            if (req.has_header("Content-Type") && req.get_header_value("Content-Type") == "text/plain") {

              const char* dirname = "received_files";
              int status = mkdir(dirname, 0777); // 使用 mkdir 函数创建目录，权限设置为 0777（可读、可写、可执行)
              if (status == 0) {
                dlog::LogInfo(__func__, "Directory created successfully.");
              } else {
                dlog::LogWarn(__func__, "Failed to create directory");
              }

              std::ofstream outFile(std::string(dirname) + "/" + dlog::get_time_now());
              //dlog::LogInfo(__func__, "header: ");
              printRequestInfo(req);
                if (outFile) {
                    outFile << req.body;
                    outFile.close();
                    res.set_content("kan dao zhe ge ,shuo ming ce shi cheng gong le", "text/plain");
                } else {
                    res.status = 500;
                    res.set_content("Failed to save file", "text/plain");
                }
              } else {
                res.status = 400;
                res.set_content("Invalid content type", "text/plain");
              }
          });

        svr.Post("/getJob", [&job](const httplib::Request& req, httplib::Response& res){
          res.set_header("Acces-Control-Allow-Orign", "*");

          assert(req.path == "/getJob");
          if(job.size() != 0){
            res.set_content(job.front(), "text/plain");
            dlog::LogInfo(__func__, "job sent: ", job.front());
            job.pop();
            dlog::LogInfo(__func__, "receive getJob request and sent");
          }
          else{
            dlog::LogWarn(__func__, "no current job !");
            res.set_content("null", "text/plain");
          }
        });

        svr.Post("/sendFilesInfo", [](const httplib::Request& req, httplib::Response& res){
          res.set_header("Acces-Control-Allow-Orign", "*");
          printRequestInfo(req);
          res.set_content("shou dao files info le", "text/plain");
          
        });

        svr.Post("/sendFile", [&buffer](const httplib::Request& req, httplib::Response& res){
          dlog::LogInfo(__func__, "receiving ...");
          buffer += req.body;
          res.set_content("part get", "text/plain");
        });

        svr.Post("/jobDone", [&end_flag](const httplib::Request& req, httplib::Response& res){
          dlog::LogInfo(__func__, "finifshed...");
          end_flag = true;
          res.set_content("finished", "text/plain");
        });


        svr.Post("/chat", [](const httplib::Request& req, httplib::Response& res){
          //dlog::LogInfo(__func__, "finifshed...");
          std::cout<<dlog::get_time_now()<<"  recv msg:"<<std::endl<<req.body<<std::endl;
          std::string answord{};
          std::cout<<"type return word:"<<std::endl;
          std::getline(std::cin, answord);
          std::cout<<"answord = "<<answord<<std::endl;
          //std::string ackstr = req.body.substr(req.body.find("ack=") + 4);
          //int ack = stoi(ackstr);
          //res.set_content(answord + "ack=" + std::to_string(ack), "text/plain");
          res.set_content(answord, "text/plain");
        });


        svr.Post("/fileDone", [&buffer, store_path](const httplib::Request& req, httplib::Response& res){
          dlog::LogInfo(__func__, " end tag");
          std::ofstream file(store_path + "/" + req.body);
          dlog::LogInfo(__func__, "REoF:" , req.body);
          if(file.is_open()){
            file << buffer ;
            file.close();
          }
          else{
            dji::dlog::LogWarn(__func__, "file not open");
          }
          res.set_content("get", "text/plain");
          buffer.clear();
        });

        //if (svr.listen("172.21.204.118", 8080)) {
        if (svr.listen("localhost", 8080)) {
            dlog::LogInfo("server", "Server is running on port 9191...");
          } else {
            dlog::LogWarn("server", "Failed to start server.");
          }
      });
      //if(unit1->joinable()){

      
      std::cout<<"excetuing..."<<std::endl;
      while(1){
        if(end_flag){
          svr.stop();
          unit1->join();
          break;
        }
      }
      end_flag = false;
      std::cout<<"closing..."<<std::endl;
    }

    
  }

  // job.emplace("getFilesInfo");
  // job.emplace("getFiles");

  return 0;
  
}