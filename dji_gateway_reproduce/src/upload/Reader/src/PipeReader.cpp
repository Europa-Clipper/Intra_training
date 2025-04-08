#include "../include/PipeReader.h"

namespace dji{
  namespace gateway{

    int PipeReader::Next(){
      if(!ifst){
        dlog::LogWarn(__func__, "invalid ifst");
        return -1;
      }

      if(finished)return -1;

      char buffer[part_size+1];
      if(ifst->Read(buffer, part_size) != 0){
        dlog::LogWarn(__func__, "read fial");
        return -1;
      }
      finished_size = ifst->Tell();
      if(finished_size == totol_size)finished = true;
      //std::cout<<"finisher size = "<<finished_size<<std::endl;
      readcache.assign(buffer, part_size);
      //dlog::LogInfo(__func__, readcache);
      return 0;
    }

    PipeReader::PipeReader(std::string path, std::int64_t totolSzie, std::int64_t partSize)
     : totol_size(totolSzie), part_size(partSize){
      ifst = new IFStream(path);
      dlog::LogInfo(__func__, "pip reader set");
      return;
    }

    PipeReader::~PipeReader(){
      if(ifst)delete(ifst);
    }

    std::string PipeReader::getReadCache(){
      return readcache;
    }

  }
}