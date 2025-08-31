#ifndef IFSTREAM_H
#define IFSTREAM_H

#include <cstdint>
#include <iostream>
#include <stdio.h>
#include <string>

#include "dlog.h"

namespace dji {
  class IFStream {
  private:
    void* _p;
    std::string _path{""};

  public:
    //std::int64_t finishedSize{0};
    
    IFStream(std::string path);
    ~IFStream();

    int64_t Tell();
    int64_t Read(char* buff, size_t size, int64_t totolSize, int64_t& finishedSize);
    
    int release_p(); //tbd
    int openfile(); //tbd
    int synfile(); //tbd

  };
  
}

#endif