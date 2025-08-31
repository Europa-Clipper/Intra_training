#ifndef PIPEREADER_H
#define PIPEREADER_H

#include "ContentReader.h"
#include "../../../utility/include/IFStream.h"
#include <cstdint>


namespace dji{
  namespace gateway{
		class PipeReader : public ContentReader {
			public:

			IFStream* ifst;
			std::int64_t finished_size{0};

			int Next() override;
			bool releaseCache();
			std::string getReadCache();
			bool reset(std::string path, std::int64_t totolSzie, std::int64_t partSize);

			PipeReader(std::string path, std::int64_t totolSzie, std::int64_t partSize);//(path, totolSize, totolSize/6 + 1)
			~PipeReader()=default;
			

			private:

			std::int64_t totol_size;
			std::int64_t part_size;
			
			std::string readcache{};

			bool finished{false};

		};



}  //namenaspcae gateway
}  //namespace dji

#endif