#include <cstdint>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <dirent.h>

#include "uploadMgr.h"


namespace dji{
  namespace gateway {

    uploadMgr::uploadMgr(std::string &path){
        DIR* dir = opendir(path.c_str());
        if (dir == nullptr) {
          dlog::LogWarn(__func__, "null dir: ", path, dir);
          return;
        }
    
        struct dirent* entry;
        int count{0};  
        while ((entry = readdir(dir)) != nullptr) {
          if (entry->d_type == DT_REG) {
              ++count;
              dlog::LogInfo(__func__, "found:" , std::string(entry->d_name));
            }
        }
        closedir(dir);
        valid = true;
        content_path = path;
        file_count = count;
    }

    int uploadMgr::deal_content(){
      DIR* dir = opendir(content_path.c_str());
      dlog::LogInfo(__func__, "content path :" , content_path);
      struct dirent* entry;
      
      while((entry = readdir(dir)) != nullptr){
        //if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))continue;

        if(entry->d_type == DT_REG){
          std::string full_path = content_path + "/" + std::string(entry->d_name);
          std::ifstream ifs(full_path);
          ifs.seekg(0, std::ios::end);
          std::streamsize size = ifs.tellg();
          ifs.close();
          if(static_cast<int64_t>(size) != -1)upload_list.emplace(std::string(entry->d_name), static_cast<int64_t>(size));
        }
        else if(entry->d_type == DT_DIR){
          dir_list.push_back(entry->d_name);
        }
        
      }

      if(upload_list.size() == 0){
        return -1;
      }

      for(auto it: upload_list){
        //std::cout<<"name : "<<it.first<<"  size : "<<it.second<<std::endl;
        dlog::LogInfo(__func__, "file name : " , it.first , "   size : " , it.second);
      }

      return 0;
    }

    bool uploadMgr::isReqValid(){
      dlog::LogInfo(__func__, "valid is " , std::to_string(valid));
      return valid;
    }

    // int upload_files_in_list(){
      
    // }

    bool uploadMgr::fileCanUpload(const std::string& name){
      if(name.find(".txt") != std::string::npos|| 
      name.find(".conf") != std::string::npos|| 
      name.find(".md")!= std::string::npos|| 
      name.find(".sh")!=std::string::npos) {
        if(upload_list.find(name + ".upload") == upload_list.end())return true;
      }
      return false;
    }

  }
}