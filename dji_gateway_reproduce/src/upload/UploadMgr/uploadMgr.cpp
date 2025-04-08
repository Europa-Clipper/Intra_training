#include <cstdint>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <dirent.h>

#include "uploadMgr.h"


namespace dji{
  namespace gateway {

    uploadMgr::uploadMgr(std::string path){
        DIR* dir = opendir(path.c_str());
        if (dir == nullptr) {
          dlog::LogWarn(__func__, "null dir!");
          return;
        }
    
        struct dirent* entry;
        int count{0};  
        while ((entry = readdir(dir)) != nullptr) {
          if (entry->d_type == DT_REG) {
              ++count;
              dlog::LogInfo(__func__, "found file :" + std::string(entry->d_name));
            }
        }
        closedir(dir);
        valid = true;
        content_path = path;
        file_count = count;
    }

    int uploadMgr::deal_content(){
      DIR* dir = opendir(content_path.c_str());
      dlog::LogInfo(__func__, "content path :" + content_path);
      struct dirent* entry;
      
      while((entry = readdir(dir)) != nullptr){
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))continue;
        std::string full_path = content_path + "/" + std::string(entry->d_name);
        std::ifstream ifs(full_path);
        ifs.seekg(0, std::ios::end);
        std::streamsize size = ifs.tellg();
        ifs.close();

        if(static_cast<int64_t>(size) != -1)upload_list.emplace(std::string(entry->d_name), static_cast<int64_t>(size));
      }

      if(upload_list.size() == 0){
        return -1;
      }

      for(auto it: upload_list){
        //std::cout<<"name : "<<it.first<<"  size : "<<it.second<<std::endl;
        dlog::LogInfo(__func__, "file name : " + it.first + "   size : " + std::to_string(it.second));
      }

      return 0;
    }

    bool uploadMgr::isReqValid(){
      dlog::LogInfo(__func__, "valid is " + std::to_string(valid));
      return valid;
    }

  }
}