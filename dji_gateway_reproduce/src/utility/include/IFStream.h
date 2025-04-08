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
    IFStream(std::string path);
    ~IFStream();

    int64_t Tell();
    int64_t Read(char* buff, size_t size);
    
    int release_p(); //TBD
    
    int openfile(); //TBD
    int synfile(); //TBD

  };
  
}

#endif