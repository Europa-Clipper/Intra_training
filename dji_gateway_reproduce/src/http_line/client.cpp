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

    //httplib::Client cli("4005xz95wp70.vicp.fun", 80);
		httplib::Client cli("localhost", 8080);

    // std::ifstream file("/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/upload_test/testfile571.conf");
    // if (!file) {
    //   dlog::LogWarn(__func__, "Failed to open file");
    //   return 1;
    // }

    // //std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // file.close();
    std::string fileContent{"test wheather can get the job todo"};

    while(1){
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


    // auto res = cli.Post("/upload", fileContent, "text/plain");

    // if (res) {
    //   dlog::LogInfo(__func__, "connect success, res stauts: ", res->status);
    //   dlog::LogInfo(__func__, "res body is: ", res->body);
    // } else {
		// dlog::LogWarn(__func__, "request failed : ", res.error());
    // }

    return 0;
}