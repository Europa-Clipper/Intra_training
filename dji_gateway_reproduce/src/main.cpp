#include <cstdint>
#include <iostream>
//#include <thread>

#include "utility/include/dlog.h"
#include "utility/include/IFStream.h"

int main(){
  std::cout<<"loop start"<<std::endl;
  dji::dlog::LogInfo(__func__, "running dji_gateway....");
  dji::dlog::LogWarn(__func__, "upload test fail");

  dji::IFStream ifst("/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/testfile.conf");
  char buffer[150];

  int64_t ret = ifst.Read(buffer, 98);
  std::cout<<"read ret = "<<ret<<std::endl;
  std::cout<<"read pos: "<<ifst.Tell()<<std::endl;
  for(int i = 0; i < 98; ++i){
    std::cout<<buffer[i];
  }
  std::cout<<std::endl;
  ifst.release_p();

  // while(1){
  //   dji::dlog::LogInfo(__func__, "clock tick");
  //   std::this_thread::sleep_for(std::chrono::seconds(1));
  // }
  delete &ifst;
  return 0;
}