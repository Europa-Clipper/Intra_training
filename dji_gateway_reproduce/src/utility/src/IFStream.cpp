#include "../include/IFStream.h"
#include <cstddef>
#include <cstdint>
#include <fstream>

namespace dji{
  
  IFStream::IFStream(std::string path){

    if(path.size() == 0){
      dlog::LogWarn(__func__, "NULL PATH");
    }
    dlog::LogInfo(__func__, "path set");
    _path = path;

    #ifdef __QNX__
    /**/
    #else
    std::ifstream* stream = new std::ifstream(_path);
    if(stream->is_open()){
      dlog::LogInfo(__func__, "file set");
    }
    else{
      dlog::LogWarn(__func__, "file set fail");
      delete(stream);
      return;
    }
    _p = static_cast<void*>(stream);
    #endif
  }

  IFStream::~IFStream(){
    delete(static_cast<std::ifstream*>(_p));
    return;
  }

  int64_t IFStream::Tell(){
    if(_path.size() == 0 || !_p){
      dlog::LogWarn(__func__, "path or file invalid");
      return -1;
    }

    #ifdef __QNX__
    // FILE* p = (FILE*)_p;
    return (int64_t)::ftell((FILE*)_p);
    #else
    //std::ifstream* fd = static_cast<std::ifstream*>(_p);
    //dlog::LogInfo(__func__, "path or file valid");
    if(!static_cast<std::ifstream*>(_p)->is_open()){
      dlog::LogWarn(__func__, "file not open");
    }
    return static_cast<int64_t>((static_cast<std::ifstream*>(_p))->tellg());
    #endif
  }

  int64_t IFStream::Read(char* buff, size_t size){
    if(_path.size() == 0 || !_p){
      dlog::LogWarn(__func__, "path or file invalid");
      return -1;
    }

    if(!buff || size <= 0){
      dlog::LogWarn(__func__, "buffer or size invalid");
      return -1;
    }

    #ifdef __QNX__
    /**/
    #else
    std::ifstream* ifs = static_cast<std::ifstream*>(_p);
    if(ifs->is_open()){
      std::int64_t remain_size = size;
      std::int64_t read_offset{0};
      while (remain_size != 0 && !ifs->eof()) {
        ifs->read(buff + read_offset, remain_size);
        remain_size -= static_cast<int64_t>(ifs->gcount());
        read_offset += static_cast<int64_t>(ifs->gcount());
        //std::cout<<"read num is"<<static_cast<int64_t>(ifs->gcount())<<std::endl;
      }
      // (static_cast<int64_t>(ifs->gcount()) != size && !ifs->eof()){
      //   return -2;
      // }
      //dlog::LogInfo(__func__, msg);
      return 0;
    }
    dlog::LogWarn(__func__, "file is not open");
    return -1;

    #endif

  }

  // int IFStream::release_p(){
  //   delete(static_cast<std::ifstream*>(_p));
  //   return 0;
  // }


}