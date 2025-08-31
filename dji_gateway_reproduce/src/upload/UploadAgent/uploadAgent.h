#include <iostream>
#include <unordered_map>

#include "../Reader/include/PipeReader.h"

namespace dji {
  namespace gateway {

    enum upload_status {
      unUpload = 0,
      uploading = 1,
      uploaded = 2,
      upload_failed = 3
    };
    
    class uploadAgent
    {
    private:
      std::unordered_map<int, upload_status>upload_records; //<id, status>
      
    public:
      uploadAgent();
      ~uploadAgent() = default;

      int update_record(int upload_id, upload_status status);

      int gen_upload_id(PipeReader pipredr);

      int make_task(PipeReader pipredr);


    };
    
    //uploadAgent::uploadAgent()
    
    
    //uploadAgent::~uploadAgent()





  }

}