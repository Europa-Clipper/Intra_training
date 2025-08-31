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
      dlog::LogWarn(__func__, "file set fail, path: " , path);
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
    // //std::ifstream* fd = static_cast<std::ifstream*>(_p);
    // //dlog::LogInfo(__func__, "path or file valid");
    // if(!static_cast<std::ifstream*>(_p)->is_open()){
    //   dlog::LogWarn(__func__, "file not open");
    //   return -1;
    // }
    // return static_cast<int64_t>((static_cast<std::ifstream*>(_p))->tellg());

    std::ifstream* ifs = static_cast<std::ifstream*>(_p);
        if (!ifs->is_open()) {
            dlog::LogWarn(__func__, "file not open");
            return -1;
        }
        try {
            //std::cout<<"now trying tell..."<<std::endl;
            return static_cast<int64_t>(ifs->tellg());
        } catch (const std::ios_base::failure& e) {
            dlog::LogWarn(__func__, "Error getting file position: " , std::string(e.what()));
            return -1;
        }
    #endif
  }

  int64_t IFStream::Read(char* buff, size_t size, int64_t totolsize, int64_t& finishedSize){
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
    if (ifs->is_open()) {
        std::int64_t remain_size = std::min(static_cast<std::int64_t>(size), totolsize - finishedSize);
        std::int64_t should_read_size = remain_size;
        std::int64_t read_offset{0};

        while (remain_size > 0) {
          ifs->read(buff + read_offset, remain_size);
          std::streamsize bytes_read = ifs->gcount();
          dlog::LogInfo(__func__, "remain size: ", remain_size, "readoffset: ", read_offset);
          if (bytes_read == 0) {
              if (ifs->eof()) {
                  dlog::LogInfo(__func__, "reached end of file ");
                  break;
              } else {
                  dlog::LogWarn(__func__, "Read error: ", _path);
                  return -1;
              }
          }

          remain_size -= bytes_read;
          read_offset += bytes_read;
        }
      dlog::LogInfo(__func__, "aft remain size: ", remain_size, "readoffset: ", read_offset);
      finishedSize += read_offset;
      return should_read_size - remain_size;
    }

    return -1;
    #endif
  }

  // int IFStream::release_p(){
  //   delete(static_cast<std::ifstream*>(_p));
  //   return 0;
  // }


}