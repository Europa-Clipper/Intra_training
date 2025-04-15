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
      //std::cout<<"before tell:"<<ifst->Tell()<<std::endl;
      int64_t ret = ifst->Read(buffer, part_size, totol_size, finished_size);
      //dlog::LogInfo(__func__, "ifst->read ret is : ", ret);
      if(ret == -1){
        dlog::LogWarn(__func__, "read fial");
        return -1;
      }
      finished_size = ifst->Tell();
      if(finished_size == -1){
        dlog::LogWarn(__func__, "tell wrong ");
        return -1;
      }
      //std::cout<<__func__<<"  finished size: "<<finished_size<<std::endl;
      if(finished_size >= totol_size)finished = true;
      std::cout<<"aft tell= "<<finished_size<<std::endl;
      buffer[ret] = '\0';
      readcache.assign(buffer, ret);
      //dlog::LogInfo(__func__, "readcache = ", readcache);
      //dlog::LogInfo(__func__, readcache);
      return 1;
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