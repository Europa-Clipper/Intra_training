#include <cstdint>
#include <iostream>
#include <thread>

#include "utility/include/dlog.h"
#include "upload/Reader/include/PipeReader.h"
#include "upload/UploadMgr/uploadMgr.h"

int main(){
  std::cout<<"loop start"<<std::endl;
  dji::dlog::LogInfo(__func__, "running dji_gateway....");
  dji::dlog::LogWarn(__func__, "upload test fail");

  std::string path{"/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/testfile.conf"};

  //dji::IFStream ifst(path);
  //char buffer[150];

  // int64_t ret = ifst.Read(buffer, 98);
  // std::cout<<"read ret = "<<ret<<std::endl;
  // std::cout<<"read pos: "<<ifst.Tell()<<std::endl;
  // for(int i = 0; i < 98; ++i){
  //   std::cout<<buffer[i];
  // }
  // std::cout<<std::endl;
  // ifst.release_p();

  // while(1){
  //   dji::dlog::LogInfo(__func__, "clock tick");
  //   std::this_thread::sleep_for(std::chrono::seconds(1));
  // }

  dji::gateway::PipeReader pipread(path, 600, 100);
  int n = 1;
  while(!pipread.Next()){
    //dji::dlog::LogInfo(__func__, "now printing new block");
    std::cout<<"now printing block "<< n <<" : "<<std::endl;
    std::cout<<pipread.getReadCache()<<std::endl<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ++n;
  }
  dji::dlog::LogInfo(__func__, "print done");

  dji::gateway::uploadMgr upmgr("/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/upload_test");
  if(upmgr.isReqValid()){
    upmgr.deal_content();

    //for()
  }


  return 0;
}