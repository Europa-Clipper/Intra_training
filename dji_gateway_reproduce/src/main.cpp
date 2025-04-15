#include <cstdint>
#include <iostream>
#include <thread>
#include <fstream>

#include "utility/include/dlog.h"
#include "upload/Reader/include/PipeReader.h"
#include "upload/UploadMgr/uploadMgr.h"

int main(){
  std::cout<<"loop start"<<std::endl;
  int a = 10909;
  double b = 378.89;
  dji::dlog::LogInfo(__func__, "running dji_gateway.... confirm code is " , a, b, "fail ret is :", 12446464097);
  dji::dlog::LogWarn(__func__, "upload test fail");

  std::string path{"/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/testfile.conf"};

  dji::gateway::PipeReader pipread(path, 600, 101);
  int n = 1;
  while(pipread.Next() != -1){
    //dji::dlog::LogInfo(__func__, "now printing new block");
    std::cout<<"now printing block "<< n <<" : "<<std::endl;
    std::cout<<pipread.getReadCache()<<std::endl<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ++n;
  }
  dji::dlog::LogInfo(__func__, "print done");

  // dji::gateway::uploadMgr upmgr("/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/upload_test");
  // if(upmgr.isReqValid()){
  //   upmgr.deal_content();

  //   //for()
  // }
  // dji::gateway::uploadMgr upmgr("/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/upload_test");
	// 	if(upmgr.isReqValid()){
	// 		upmgr.deal_content();
	// 	}
	// 	else{
	// 		dji::dlog::LogWarn(__func__, "invalid uploadmgr");
	// 		return -1;
	// 	}

	// 	for(auto it = upmgr.upload_list.begin(); it != upmgr.upload_list.end(); ++it){
	// 		if((it->first.size() > 7) && ((it->first).substr(it->first.size()-7) != ".upload") && //daixiugai
	// 		(upmgr.upload_list.find(it->first + ".upload") == upmgr.upload_list.end())){

	// 			dji::gateway::PipeReader pipreadr(upmgr.content_path + "/" + it->first, it->second, (it->second / 6) + 1);
  //       dji::dlog::LogInfo(__func__, "it->second: ", it->second, (it->second / 6) + 1);
  //       int ret = pipreadr.Next();
	// 			while(pipreadr.Next()){
	// 				//std::string patch = &(pipreadr.getReadCache());
	// 				//auto res = cli.Post("/sendFile", (pipreadr.getReadCache()), "text/plain");
  //         std::this_thread::sleep_for(std::chrono::seconds(3));
	// 				dji::dlog::LogInfo(__func__, "current part readcache: ", pipreadr.getReadCache());
	// 			}
  //       dji::dlog::LogInfo(__func__, "next.ret = ", ret);
	// 			//dji::dlog::LogInfo(__func__, "readcache: ", pipreadr.getReadCache());
	// 			//auto res = cli.Post("/fileDone", it->first, "text/plain");
				
	// 			dji::dlog::LogInfo(__func__, "sent file ", it->first);

	// 			//dji::dlog::LogInfo(__func__, ".upload create path: ", upmgr.content_path + "/" + it->first + ".upload");
	// 			std::ofstream file(upmgr.content_path + "/" + it->first + ".upload");
	// 			if(file.is_open()){
	// 				file.close();
	// 				dji::dlog::LogInfo(__func__, ".upload created :", upmgr.content_path + it->first + ".upload");
	// 			}

	// 		}
	// 		else{
	// 			dji::dlog::LogInfo(__func__, "skipping ", it->first, "... ");
	// 		}
	// 	}


  return 0;
}