#include <cstdint>
#include <iostream>
#include <thread>
#include <fstream>
#include <filesystem>
#include <queue>


#include "utility/include/dlog.h"
#include "upload/Reader/include/PipeReader.h"
#include "upload/UploadMgr/uploadMgr.h"
// #include "resource_monitor/include/sys_monit_server.h"
// #include "resource_monitor/include/sys_monit_client.h"
#include "resource_monitor/include/ws_client.h"
#include "resource_monitor/include/ws_server.h"
#include "../3rdParty/cmdline-master/cmdline-master/cmdline.h"
#include "../3rdParty/cpp-httplib-master/httplib.h"
#include "http_line/server.h"

// #include "chatServer/include/serv.h"

namespace fs = std::filesystem;

std::atomic<bool> g_terminate(false);
static bool program_running = true;
static bool running = true;

// void handle_signal(int sig) {
//     std::cout << "\n收到到退出信号，正在关闭程序..." << std::endl;
//     running = false;
//     stop_server();  // 通知服务器停止
//     stop_client();  // 通知客户端停止
// }


int ClientDealWithResBody(std::string &body, httplib::Client& cli, fs::path& path){
	if(body == "null")return 0;
  dji::dlog::LogInfo(__func__, "cwd is ", path);

	if(body == "getFilesInfo"){
		std::string ppa = path.string();
		dji::gateway::uploadMgr upmgr((ppa));
		if(upmgr.isReqValid()){
			upmgr.deal_content();
		}
		else{
			dji::dlog::LogWarn(__func__, "invalid uploadmgr");
			return -1;
		}
		
		std::string files_info;
		for(auto it = upmgr.upload_list.begin(); it != upmgr.upload_list.end(); ++it){
			files_info += ((it->first + "  size: " + std::to_string(it->second)) + "\n");
		}

		auto res1 = cli.Post("/sendFilesInfo", files_info, "text/plain");
		if(res1){
			//dji::dlog::LogInfo(__func__, "send filesInfo res back: ", res1->status);
			auto res = cli.Post("/jobDone", "job done", "text/plain");
			return 0;
		}
		else{
			dji::dlog::LogWarn(__func__, "send filesInfo fail");
			return -1;
		}
	}
  else if(body == "getDirInfo"){
    std::string ppa = path.string();
    dji::gateway::uploadMgr upmgr(ppa);
		if(upmgr.isReqValid()){
			upmgr.deal_content();
		}
		else{
			dji::dlog::LogWarn(__func__, "invalid uploadmgr");
			return -1;
		}

    std::string dir_infos{""};
    for(std::string str : upmgr.dir_list){
      dir_infos += (str + "\n");
    }

    httplib::Result res = cli.Post("/sendDirInfo", dir_infos, "text/plain");
    if(res){
			//dji::dlog::LogInfo(__func__, "send filesInfo res back: ", res1->status);
			auto res11 = cli.Post("/jobDone", "job done", "text/plain");
			return 0;
		}
		else{
			dji::dlog::LogWarn(__func__, "send dirInfo fail");
			return -1;
		}

  }
  else if(body[0] =='c' && body[1] == 'd'){
    dji::dlog::LogInfo(__func__, "change dir job");
    std::string tar{""};
    for(int i = 2; i < body.size(); ++i){
      if(body[i] != ' '){
        tar = body.substr(i, body.size()-i);
        break;
      }
    }
    try
    {
      fs::path tmp = fs::canonical(path / tar);
      path = tmp;
    }
    catch(const std::exception& e)
    {
      //std::cerr << e.what() << '\n';
      dji::dlog::LogWarn(__func__, "canoical error ", e.what());
    }
    
    //path = fs::canonical(path / tar);
    httplib::Result res = cli.Post("/changedDir", path.string(), "text/plain");
    if(res){
			//dji::dlog::LogInfo(__func__, "send filesInfo res back: ", res1->status);
			auto res11 = cli.Post("/jobDone", "job done", "text/plain");
			return 0;
		}
		else{
			dji::dlog::LogWarn(__func__, "send dirInfo fail");
			return -1;
		}


  }
	else if(body == "getFiles"){
    std::string ppa = path.string();
		dji::gateway::uploadMgr upmgr(ppa);
		if(upmgr.isReqValid()){
			upmgr.deal_content();
		}
		else{
			dji::dlog::LogWarn(__func__, "invalid uploadmgr");
			return -1;
		}

		for(auto it = upmgr.upload_list.begin(); it != upmgr.upload_list.end(); ++it){
			std::string name(it->first);

			// (it->first.size() > 7) && ((it->first).substr(it->first.size()-7) != ".upload") && //daixiugai
			// (upmgr.upload_list.find(it->first + ".upload") == upmgr.upload_list.end())

			//dji::dlog::LogInfo(__func__, "sub-4:", name.substr(name.size() - 4));
			if((name.size() >= 5) && (name.substr(name.size() - 4) == ".txt"|| name.substr(name.size() - 5) == ".conf" ) && (upmgr.upload_list.find(name + ".upload") == upmgr.upload_list.end())){

				dji::gateway::PipeReader pipreadr(upmgr.content_path + "/" + it->first, it->second, it->second / 6 + 1);
				int times{0};
				while(pipreadr.Next()!= -1){
					//std::string patch = &(pipreadr.getReadCache());
					std::this_thread::sleep_for(std::chrono::seconds(1));
					auto res = cli.Post("/sendFile", (pipreadr.getReadCache()), "text/plain");
					//dji::dlog::LogInfo(__func__, "current sent part readcache: ", pipreadr.getReadCache());
					++times;
					std::cout<<"pao le "<<times<<" ci"<<std::endl;
				}
				auto res = cli.Post("/fileDone", it->first, "text/plain");
				
				dji::dlog::LogInfo(__func__, "sent file", it->first);

				std::ofstream file(upmgr.content_path + "/" + it->first + ".upload");
				if(file.is_open()){
					file.close();
					dji::dlog::LogInfo(__func__, ".upload created :", upmgr.content_path + "/" + it->first + ".upload");
				}

			}
			else{
				dji::dlog::LogInfo(__func__, "skipping ", it->first, "... ");
			}
		}

	}
	auto res = cli.Post("/jobDone", "job done", "text/plain");

	return 0;
}


int main(int argc, char* argv[]){
	using namespace dji;
	using namespace gateway;

  cmdline::parser parser;

  parser.add("help", 'h', "print help info");
  parser.add("server", 's', "server mode");
  parser.add("client", 'c', "client mode");
  parser.add<int>("port", 'p', "端口号", false, 8080);
  parser.add<std::string>("addr", 'a', "ip address", false, "localhost");
  parser.parse_check(argc, argv);

  int port = 8080;
  std::string addr = "localhost";

  if(parser.exist("port")){
    port = parser.get<int>("port");
  }
  if(parser.exist("addr")){
    addr = parser.get<std::string>("addr");
  }

	// std::cout<<"loop start"<<std::endl;
  dlog::LogInfo(__func__, "running log test...");
  int a = 10909;
  double b = 378.89;
  dlog::LogInfo(__func__, "running dji_gateway.... confirm code is " , a, b, "fail ret is :", 12446464097);
  dlog::LogWarn(__func__, "warntest,viwviwejihiwejbvuwi");

  std::string path{"/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce"};
  dlog::LogInfo(__func__, "running readertest...");
  // PipeReader pipread(path, 600, 101);
  // int n = 1;
  // while(pipread.Next() != -1){
  //   //dji::dlog::LogInfo(__func__, "now printing new block");
  //   //std::cout<<"now printing block "<< n <<" : "<<std::endl;
	// 	dji::dlog::LogInfo(__func__, "now printing block...");
  //   std::cout<<pipread.getReadCache()<<std::endl<<std::endl;
  //   std::this_thread::sleep_for(std::chrono::seconds(1));
  //   ++n;
  // }
  dji::gateway::uploadMgr upmgr(path); 
  if(upmgr.isReqValid()){
    upmgr.deal_content();

    //dji::gateway::PipeReader pipreadr(upmgr.content_path + "/" + it->first, it->second, it->second / 6 + 1);
    std::unordered_map<std::string, int64_t>::iterator it;
    for(it = upmgr.upload_list.begin(); it != upmgr.upload_list.end(); ++it){
      if(not upmgr.fileCanUpload(it->first))continue;
      dji::gateway::PipeReader pipreadr(upmgr.content_path + "/" + it->first, it->second, it->second / 6 + 1);
      //dji::dlog::LogInfo(__func__, it->first, it->second);
      while(pipreadr.Next()!= -1){
        dji::dlog::LogInfo(__func__, it->second);
        //dlog::LogInfo(__func__, (pipreadr.getReadCache()));
        std::cout<<pipreadr.getReadCache()<<std::endl;
        //std::this_thread::sleep_for(std::chrono::seconds(5));
      }
    }
  }
  else {
    dlog::LogError(__func__, "upmgr not valid");
  }
  dji::dlog::LogInfo(__func__, "print done, n = ");


  if(parser.exist("help")){
    std::cout<<"============================ help ================================="<<std::endl<<
    "this is a test gateway, wo yao qu momenta"<<std::endl;
  }

  if(parser.exist("client")){
    //dlog::LogInfo(__func__, "============================ client mode ============================");
    std::cout<<"============================ client mode initializing ============================"<<std::endl;

    httplib::Client cli(addr, port);
    std::string fileContent{"test wheather can get the job todo"};
    fs::path cur_dir = fs::current_path();

    while(1){
				dlog::LogInfo(__func__, "no chat mode");
				auto res = cli.Post("/getJob", fileContent, "text/plain");

				if (res) {
					dlog::LogInfo(__func__, "connect success, res stauts: ", res->status, "\n", "res body is: ", res->body);
					//dlog::LogInfo(__func__, "res body is: ", res->body);
					int ret = ClientDealWithResBody(res->body, cli, cur_dir);
					dlog::LogInfo(__func__, "deal res body, ret is ", ret);
				} else {
					dlog::LogInfo(__func__, "request failed : ", res.error());
				}
				std::this_thread::sleep_for(std::chrono::seconds(3));
		}
  }
  else if(parser.exist("server")){
    //dlog::LogInfo(__func__, "============================ server modeinitializing ============================");
    std::cout<<"============================ server mode initializing ============================"<<std::endl;
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
                <<"||    chat server     ||   press 4"<<std::endl
                <<"||       exit         ||   press 5"<<std::endl
                <<"||    get dir info    ||   press 6"<<std::endl
                <<"||    change dir      ||   press cd + dir"<<std::endl
                <<"||   monitor server   ||   press 7"<<std::endl
                <<"||   monitor client   ||   press 8"<<std::endl;

      std::getline(std::cin, cmd);

      if(cmd == "1"){
        job.emplace("getFilesInfo");
      }
      else if(cmd == "2"){
        job.emplace("getFiles");
      }
      else if(cmd == "3"){

        // std::unique_ptr<std::thread> chatThread = std::make_unique<std::thread>([&svr](){
        //   bool getrecv{false};
        //   std::string content{};
          
        //   svr.Post("/chat", [&content](const httplib::Request& req, httplib::Response& res){
        //     std::cout<<dji::dlog::get_time_now()<<"  "<<req.body<<std::endl<<"type retrun:"<<std::endl;
        //     std::getline(std::cin, content);
        //     std::cout<<"content is :"<<content<<std::endl;
        //     // int ackbc = stoi(req.body.substr(req.body.find("ack=")));
        //     res.set_content(content, "text/plain");
        //   });

        //   svr.listen("localhost", 8080);
        //   //svr.listen("172.21.204.118", 8080);
        // });
        // if(chatThread->joinable()){
        //   chatThread->join();
        // }



        // using namespace websktcli;
        // using websocketpp::connection_hdl;

        // //string uri = "ws://" + string(argv[1]) + ":" + string(argv[2]);
        // //std::string uri = "ws://" + std::string(argv[1]) + ":" + std::string(argv[2]);
        // std::string uri = "ws://" + addr + std::to_string(port);
        // client ws_client;

        // try {

        //     ws_client.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect);
        //     ws_client.set_error_channels(websocketpp::log::elevel::rerror | websocketpp::log::elevel::fatal);

        //     ws_client.init_asio();

        //     ws_client.set_message_handler(std::bind(&on_message, &ws_client, std::placeholders::_1, std::placeholders::_2));
        //     ws_client.set_open_handler(std::bind(&on_open, &ws_client, std::placeholders::_1));
        //     ws_client.set_close_handler(std::bind(&on_close, &ws_client, std::placeholders::_1));
        //     ws_client.set_fail_handler(std::bind(&on_fail, &ws_client, std::placeholders::_1));

        //     websocketpp::lib::error_code ec;
        //     auto con = ws_client.get_connection(uri, ec);
        //     if (ec) {
        //         std::cerr << "connection error: " << ec.message() << std::endl;
        //         return 1;
        //     }
        //     ws_client.connect(con);

        //     std::thread input_thread(client_input_thread, &ws_client);

        //     ws_client.run();

        //     if (input_thread.joinable()) {
        //         input_thread.join();
        //     }
        // } catch (std::exception const& e) {
        //     std::cerr << "error: " << e.what() << std::endl;
        // }


      }
      else if(cmd == "4"){
        // std::unique_ptr<std::thread> chatServerThread = std::make_unique<std::thread>([&svr](){
        //   int id_count{0};
        //   std::vector<chatmsg> msgline{};
        //   svr.Post("/serverchat", [&msgline](const httplib::Request& req, httplib::Response& res){
        //     msgline.emplace_back(string_to_chatmsg(req.body)/**body to chatmsg***/);
        //   });

        //   svr.Post("/startchat", [&id_count, &msgline](const httplib::Request& req, httplib::Response& res){
        //     res.set_content("id=" + std::to_string(id_count) + "//curnum=" + std::to_string(msgline.size()), "text/plain"); //ex: "id=2//curnum=30"
        //     ++id_count;
        //   });
        //   std::string strr{"id=7curnum=201cnt=zheshiyigeceshi,,kankanshifouzhengquetime=2025.4.16 15:57"};
        //   std::cout<<"strr="<<strr<<std::endl;
        //   string_to_chatmsg(strr);
        // });
        
        // chatServerThread->join();



    //     using namespace websktserv;
    //     //typedef websocketpp::server<websocketpp::config::asio> server;
    //     //using websocketpp::connection_hdl;

    //     std::set<connection_hdl, std::owner_less<connection_hdl>> connections;
    //     server ws_server;

    //     try {

    //     ws_server.set_access_channels(websocketpp::log::alevel::none);
    //     ws_server.set_error_channels(websocketpp::log::elevel::none);

    //     ws_server.init_asio();

    //     ws_server.set_open_handler(std::bind(&on_open, &ws_server, std::placeholders::_1));
    //     ws_server.set_close_handler(std::bind(&on_close, &ws_server, std::placeholders::_1));
    //     ws_server.set_message_handler(std::bind(&on_message, &ws_server, std::placeholders::_1, std::placeholders::_2));

    //     ws_server.listen(8080);

    //     dji::dlog::LogInfo(__func__, "svr listen on port 8080\n","type msg to send to clients, type 'exit' to quit");

    //     ws_server.start_accept();

    //     std::thread input_thread(server_input_thread, &ws_server);

    //     ws_server.run();

    //     input_thread.join();
    // } catch (std::exception const& e) {
    //     std::cerr << "error: " << e.what() << std::endl;
    // } catch (...) {
    //     std::cerr << "unknow error" << std::endl;
    // }


      
      }
      else if(cmd == "5"){
        dji::dlog::LogInfo(__func__, "exit flag true, exiting...");
        break;
      }
      else if(cmd == "6"){
        //dji::dlog::LogInfo(__func__, "")
        job.emplace("getDirInfo");
      }
      else if(cmd[0] == 'c' && cmd[1] == 'd'){
        job.emplace(cmd);
      }
      else if(cmd == "7"){
        
          myWebSocketServer server(8080);

          server.start();
    
          // 等待服务器停止
          //server.wait();
          
          // 停止服务器
          server.stop();
      }
      else if(cmd == "8"){

       std::string url = "ws://localhost:8080";
    
        // 创建客户端实例
        myWebSocketClient client(url);
        
        // 连接到服务器
        client.connect();
        
        // 启动交互模式
        client.startInteractiveMode();
        
        // 断开连接
        client.disconnect();

        
      }
      else{
        std::cout<<"valid input"<<std::endl;
      }
      continue;
    }
    else{
      dji::dlog::LogInfo(__func__, job.front());
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

        svr.Post("/sendDirInfo", [](const httplib::Request& req, httplib::Response& res){
          res.set_header("Access-Control-Allow-Orign", "*");
          printRequestInfo(req);
          res.set_content("get dir info", "text/plain");
        });

        svr.Post("/changedDir", [](const httplib::Request& req, httplib::Response& res){
          res.set_header("Access-Control-Allow-Orign", "*");
          printRequestInfo(req);
          res.set_content("get changed dir", "text/plain");
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      end_flag = false;
      std::cout<<"closing..."<<std::endl;
    }
  }
  return 0;
  }
	
  return 0;
}