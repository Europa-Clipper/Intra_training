#include <iostream>
#include <fstream>

#include "../../3rdParty/cpp-httplib-master/httplib.h"
#include "../utility/include/dlog.h"
#include "../upload/UploadMgr/uploadMgr.h"

int dealWithResBody(std::string body, httplib::Client& cli){
	if(body == "null")return 0;

		const int bufferSize = 1024;
    char buffer[bufferSize];
		std::string dirr;
		char* result = getcwd(buffer, bufferSize);
    if (result != nullptr) {
      std::cout << "curr dir is : " << buffer << std::endl;
			dirr = std::string(buffer);
			std::cout<<"str: "<< dirr <<std::endl;
    } else {
      std::cerr << "get fir fail" << std::endl;
    }

	if(body == "getFilesInfo"){
		
		dji::gateway::uploadMgr upmgr(dirr);
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
			//dji::dlog::LogWarn(__func__, "send filesInfo fail");
			return -1;
		}
	}
	else if(body == "getFiles"){
		dji::gateway::uploadMgr upmgr(dirr);
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

std::string checkAckFromBody(std::string body){
	//int pos = body.find("ack=");
	return body.substr(body.find("ack="));
}

int main(int argc , char* argv[]) {
    using namespace dji;

		const int bufferSize = 1024;
    char buffer[bufferSize];
		char* result = getcwd(buffer, bufferSize);
    if (result != nullptr) {
      std::cout << "curr dir is : " << buffer << std::endl;
			std::string dirr(buffer);
			std::cout<<"str: "<< dirr <<std::endl;
    } else {
      std::cerr << "get fir fail" << std::endl;
    }
		dlog::LogWarn(__func__, "reoivheoirbverbv");
    //httplib::Client cli("4005xz95wp70.vicp.fun", 80);
		httplib::Client cli("localhost", 8080);

    
    std::string fileContent{"test wheather can get the job todo"};
		if(argc > 1)std::cout<<"showing agrc"<<argc<<"  "<<argv[0]<<"  "<<argv[1]<<std::endl;
		if(argc > 1 && std::string(argv[1]) == "chat"){
			std::cout<<"chat mode"<<std::endl<<std::endl;
			
			std::unique_ptr<std::thread> thr1 = std::make_unique<std::thread>([&fileContent, &cli](){
				bool getrecv{false};
				size_t ack{0};

				while(1){
					getrecv = false;
					std::cout<<"type words: "<<std::endl<<"(type @stop@chat if want to stop chat)"<<std::endl;
					std::getline(std::cin, fileContent);
					if(fileContent == "@stop@chat")break;
					++ack;
					//auto res = cli.Post("/chat", fileContent + "//ack=" + std::to_string(++ack), "text/plain");
					// dlog::LogInfo(__func__, "msg sent");
					std::cout<<dlog::get_time_now()<<"  sending msg... ack = "<<ack<<std::endl;
					while(!getrecv){
						//auto res = cli.Post("/chat", fileContent + "//ack=" + std::to_string(ack), "text/plain");
						auto res = cli.Post("/chat", fileContent, "text/plain");
						if(res){
							//std::cout<<dlog::get_time_now()<<" recv ans:  "<<res->body<<"  ack ="<<res->body.substr(res->body.find("ack=") + 4);
							std::cout<<dlog::get_time_now()<<"  "<<res->body<<std::endl;
							if(res)getrecv = true;
						}
						// else{
						// 	while(!getrecv){
						// 		res = cli.Post("/chat", fileContent + "//ack=" + std::to_string(ack), "text/plain");

						// 		std::this_thread::sleep_for(std::chrono::milliseconds(500));
						// 	}
						// }
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
					}
					
				}
			});
			if(thr1->joinable()){
				thr1->join();
			}
		}
		else{
			while(1){
				dlog::LogWarn(__func__, "no chat mode");
				auto res = cli.Post("/getJob", fileContent, "text/plain");

				if (res) {
					dlog::LogInfo(__func__, "connect success, res stauts: ", res->status);
					dlog::LogInfo(__func__, "res body is: ", res->body);
					int ret = dealWithResBody(res->body, cli);
					dlog::LogInfo(__func__, "deal res body, ret = ", ret);
				} else {
					dlog::LogWarn(__func__, "request failed : ", res.error());
				}

				std::this_thread::sleep_for(std::chrono::seconds(2));
			}
		}


    return 0;
}