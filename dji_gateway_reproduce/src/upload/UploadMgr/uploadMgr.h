#ifndef UPLOADMGR_H
#define UPLOADMGR_H

#include <string.h>
#include <stdio.h>
#include "../Reader/include/PipeReader.h"
#include <cstdint>
#include <unordered_map>
#include "../UploadAgent/uploadAgent.h"

namespace dji {
  namespace gateway {

    class uploadMgr {
      public:
      
      //uploadAgent upldagt;

      uploadMgr(std::string path);
      ~uploadMgr() = default;

      std::string content_path{};
      int file_count;
      int uploaded_file_count{0};

      int deal_content();
      int get_upload_id();
      bool isReqValid();
      
      private:
      int upload_id;
      std::unordered_map<std::string, int64_t>upload_list;
      bool valid{false};
      

    };

  }
}

#endif