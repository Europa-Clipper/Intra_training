#ifndef UPLOADMGR_H
#define UPLOADMGR_H

#include <string.h>
#include <stdio.h>
#include <vector>
#include "../Reader/include/PipeReader.h"
#include <cstdint>
#include <unordered_map>
#include "../UploadAgent/uploadAgent.h"

namespace dji {
  namespace gateway {

    class uploadMgr {
      public:
      
      //uploadAgent upldagt;

      uploadMgr(std::string &path);
      ~uploadMgr() = default;

      std::string content_path{};
      int file_count;
      int uploaded_file_count{0};
      std::unordered_map<std::string, int64_t>upload_list; //<file name , file size>
      std::vector<std::string>dir_list;

      int deal_content();
      int get_upload_id();
      bool isReqValid();
      bool fileCanUpload(const std::string& name);
      int upload_files_in_list();
      
      private:
      int upload_id;
      
      bool valid{false};
      

    };

  }
}

#endif